#include "Renderer.h"
#include "CMap.h"
#include "CMapReader.h"
#include "DataInputStream.h"
#include "FileInputStream.h"
#include "FontEncoding.h"
#include "NameToUnicode.h"
#include "Object.h"
#include "ObjReader.h"
#include "PDF.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static CCMap *ReadCMap(IInputStream *pSource)
{
	CCMap *pCMap;
	CCMapReader *pReader;

	pCMap = new CCMap();
	pReader = new CCMapReader(pCMap);
	pReader->Read(pSource);
	delete pReader;
	return pCMap;
}

CRenderer::CRenderer(CPDF *pPDF)
{
	m_pPDF = pPDF;
}

CRenderer::~CRenderer()
{
}

void CRenderer::Render(int nPage)
{
	CDictionary *pPage;
	CArray *pMediaBox;
	double width, height;

	pPage = m_pPDF->GetPage(nPage);
	if (pPage == NULL)
		return;

	m_nPage = nPage;
	m_pResourceRoot = (CDictionary *)m_pPDF->GetObject(pPage->GetValue("Resources"));
	memset(m_pResources, 0, sizeof(CDictionary *) * RES_NUM);
	m_pCMap = NULL;
	m_bSimpleFont = true;
	m_pCodeToName = standardEncoding;
	memset(m_pDifferences, 0, sizeof(m_pDifferences));
	m_bStop = false;

	pMediaBox = (CArray *)pPage->GetValue("MediaBox");
	width = ((CNumeric *)pMediaBox->GetValue(2))->GetValue();
	height = ((CNumeric *)pMediaBox->GetValue(3))->GetValue();

	RenderPage(pPage, width, height);

	delete m_pCMap;
	delete pPage;
}

void CRenderer::Stop(void)
{
	m_bStop = true;
}

CObject *CRenderer::GetResource(Resource_t nRes, const char *pName)
{
	const char *pNames[] = {"ColorSpace", "Font", "ExtGState", "XObject"};

	if (m_pResources[nRes] == NULL && m_pResourceRoot != NULL)
		m_pResources[nRes] = (CDictionary *)m_pPDF->GetObject(m_pResourceRoot->GetValue(pNames[nRes]));
	return m_pResources[nRes] == NULL? NULL : m_pPDF->GetObject(m_pResources[nRes]->GetValue(pName));
}

void CRenderer::RenderPage(CDictionary *pPage, double dWidth, double dHeight)
{
	CObject *pObj;
	int i, n;

	pObj = m_pPDF->GetObject(pPage->GetValue("Contents"));
	if (pObj->GetType() == CObject::OBJ_STREAM)
		RenderContents((CStream *)pObj);
	else if (pObj->GetType() == CObject::OBJ_ARRAY)
	{
		n = ((CArray *)pObj)->GetSize();
		for (i = 0; i < n && !m_bStop; i++)
			RenderContents((CStream *)m_pPDF->GetObject(((CArray *)pObj)->GetValue(i)));
	}
}

void CRenderer::RenderContents(CStream *pContents)
{
	IInputStream *pSource;
	CDataInputStream *pDIS;
	CObjReader *pReader;
	unsigned int nOffset;
	int nParams;
	CObject *pObj;
	CObject *pParams[10];

	pSource = m_pPDF->CreateInputStream(pContents);
	pDIS = new CDataInputStream(pSource);
	pReader = new CObjReader(pDIS, m_pPDF->GetXref());
	nOffset = 0;
	nParams = 0;
	while (!m_bStop && pDIS->Available() > 0)
	{
		pDIS->Seek(nOffset, SEEK_SET);
		pObj = pReader->ReadObj();
		if (pObj == NULL)
			break;
		nOffset = pDIS->Position();
		if (nOffset == 14854)
			nOffset = 14854;

		if (pObj->GetType() == CObject::OBJ_OPERATOR)
		{
			RenderOperator((COperator *)pObj, pParams, nParams);
			delete pObj;
			while (nParams > 0)
				delete pParams[--nParams];
		}
		else
			pParams[nParams++] = pObj;
	}
	delete pReader;
	delete pDIS;
	delete pSource;
}

void CRenderer::RenderOperator(COperator *pOp, CObject **pParams, int nParams)
{
}

CStream *CRenderer::ChangeFont(const char *pName)
{
	const char *cstr, *subpath;
	CDictionary *pFont;
	CObject *pObj, *pEncoding;
	CStream *pFontFile;
	CArray *pDifferences;
	IInputStream *pSource;
	char file[128], name[16];
	int i, n, index;

	delete m_pCMap;
	m_pCMap = NULL;

	pFont = (CDictionary *)GetResource(FONT, pName);
	pFontFile = NULL;
	if (pFont == NULL)
		m_bSimpleFont = true;
	else
	{
		pObj = m_pPDF->GetObject(pFont->GetValue("Subtype"));
		cstr = ((CName *)pObj)->GetValue();
		m_bSimpleFont = strcmp(cstr, "Type1") == 0 || strcmp(cstr, "MMType1") == 0 || strcmp(cstr, "TrueType") == 0 || strcmp(cstr, "Type3") == 0;
		pObj = m_pPDF->GetObject(pFont->GetValue("ToUnicode"));
		if (pObj != NULL)
		{
			pSource = m_pPDF->CreateInputStream((CStream *)pObj);
			m_pCMap = ReadCMap(pSource);
			delete pSource;
		}

		pObj = m_pPDF->GetObject(pFont->GetValue("DescendantFonts"));
		if (pObj != NULL)
		{
			pObj = m_pPDF->GetObject(((CArray *)pObj)->GetValue(0));
			if (pObj != NULL)
				pObj = m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("FontDescriptor"));
		}
		if (pObj == NULL)
			pObj = m_pPDF->GetObject(pFont->GetValue("FontDescriptor"));

		if (pObj != NULL)
		{
			pFontFile = (CStream *)m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("FontFile"));
			if (pFontFile == NULL)
				pFontFile = (CStream *)m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("FontFile2"));
			if (pFontFile == NULL)
				pFontFile = (CStream *)m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("FontFile3"));
		}
	}

	if (m_pCMap == NULL && pFont != NULL)
	{
		pEncoding = m_pPDF->GetObject(pFont->GetValue("Encoding"));
		if (pEncoding != NULL)
			if (m_bSimpleFont)
			{
				m_pCodeToName = standardEncoding;
				memset(m_pDifferences, 0, sizeof(m_pDifferences));
				if (pEncoding->GetType() == CObject::OBJ_DICTIONARY)
				{
					pDifferences = (CArray *)m_pPDF->GetObject(((CDictionary *)pEncoding)->GetValue("Differences"));
					index = 0;
					n = pDifferences->GetSize();
					for (i = 0; i < n; i++)
					{
						pObj = pDifferences->GetValue(i);
						if (pObj->GetType() == CObject::OBJ_NUMERIC)
							index = ((CNumeric *)pObj)->GetValue();
						else
							m_pDifferences[index++] = ((CName *)pObj)->GetValue();
					}
					pEncoding = m_pPDF->GetObject(((CDictionary *)pEncoding)->GetValue("BaseEncoding"));
				}
				if (pEncoding != NULL)
				{
					cstr = ((CName *)pEncoding)->GetValue();
					if (strcmp(cstr, "WinAnsiEncoding") == 0)
						m_pCodeToName = winAnsiEncoding;
					else if (strcmp(cstr, "StandardEncoding") == 0)
						m_pCodeToName = standardEncoding;
					else if (strcmp(cstr, "MacRomanEncoding") == 0)
						m_pCodeToName = macRomanEncoding;
					else if (strcmp(cstr, "MacExpertEncoding") == 0)
						m_pCodeToName = macExpertEncoding;
				}
			}
			else
			{
				cstr = ((CName *)pEncoding)->GetValue();
				pObj = m_pPDF->GetObject(pFont->GetValue("DescendantFonts"));
				pObj = m_pPDF->GetObject(((CArray *)pObj)->GetValue(0));
				pObj = m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("CIDSystemInfo"));
				strcpy(name, ((CString *)m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("Registry")))->GetValue());
				strcat(name, "-");
				strcat(name, ((CString *)m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("Ordering")))->GetValue());
				sprintf(file, "/usr/share/poppler/cMap/%s/%s", strncmp(cstr, "Identity", 8) == 0? "" : name, cstr);
				pSource = new CFileInputStream(file);
				m_pCMap = ReadCMap(pSource);
				delete pSource;
				sprintf(file, "/usr/share/poppler/cMap/%s/%s-UCS2", name, name);
				pSource = new CFileInputStream(file);
				m_pCMap->Concat(ReadCMap(pSource));
				delete pSource;
			}
	}

	return pFontFile;
}

void CRenderer::RenderText(CString *pString)
{
	const char *cstr, *name;
	const unsigned char *ustr;
	wchar_t *wstr;
	char *str;
	int i, nLen;
	SNameToUnicode *pNameToUnicode;

	cstr = pString->GetValue();
	nLen = pString->GetLength();
	ustr = (const unsigned char *)cstr;
	wstr = new wchar_t[nLen + 1];
	i = 0;
	if (m_pCMap != NULL)
	{
		if (m_bSimpleFont)
			while (ustr < (const unsigned char *)cstr + nLen)
			{
				wstr[i] = m_pCMap->Get(*ustr++);
				if (wstr[i] == 0x00A0)  //WORKAROUND: nbsp?
					wstr[i] = ' ';
				++i;
			}
		else
			while (ustr < (const unsigned char *)cstr + nLen)
			{
				wstr[i] = m_pCMap->Get((*ustr << 8) | *(ustr + 1));
				if (wstr[i] == 0x00A0)  //WORKAROUND: nbsp?
					wstr[i] = ' ';
				++i;
				ustr += 2;
			}
	}
	else
		while (ustr < (const unsigned char *)cstr + nLen)
		{
			name = m_pDifferences[*ustr];
			if (name == NULL)
				name = m_pCodeToName[*ustr];
			++ustr;
			if (name != NULL)
				for (pNameToUnicode = nameToUnicode; pNameToUnicode->name != NULL; ++pNameToUnicode)
					if (strcmp(name, pNameToUnicode->name) == 0)
					{
						wstr[i++] = pNameToUnicode->unicode;
						break;
					}
		}
	if (i > 0)
	{
		wstr[i] = L'\0';
		str = new char[i * 6 + 1];
		str[0] = '\0';
		wcstombs(str, wstr, i * 6 + 1);
		RenderString(str);
		delete[] str;
	}
	delete[] wstr;
}

void CRenderer::RenderString(const char *str)
{
}
