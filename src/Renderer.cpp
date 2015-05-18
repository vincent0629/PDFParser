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
#include <assert.h>

using namespace std;

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

class CFontData
{
public:
	bool m_bSimpleFont;
	CCMap *m_pCMap;
	const char **m_pCodeToName;
	const char *m_pDifferences[256];
	IInputStream *m_pFontFile;

	CFontData();
	~CFontData();
};

CFontData::CFontData()
	: m_bSimpleFont(true)
	, m_pCMap(NULL)
	, m_pCodeToName(standardEncoding)
	, m_pFontFile(NULL)
{
	memset(m_pDifferences, 0, sizeof(m_pDifferences));
}

CFontData::~CFontData()
{
	delete m_pCMap;
	delete m_pFontFile;
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
	map<CObject *, CFontData *>::iterator it;

	pPage = m_pPDF->GetPage(nPage);
	if (pPage == NULL)
		return;

	m_nPage = nPage;
	m_pResourceRoot = (CDictionary *)m_pPDF->GetObject(pPage->GetValue("Resources"));
	memset(m_pResources, 0, sizeof(CDictionary *) * RES_NUM);
	m_bStop = false;

	pMediaBox = (CArray *)pPage->GetValue("MediaBox");
	if (!pMediaBox)
	{
		width = 600;
		height = 800;
	}
	else
	{
		width = ((CNumeric *)pMediaBox->GetValue(2))->GetValue();
		height = ((CNumeric *)pMediaBox->GetValue(3))->GetValue();
	}

	RenderPage(pPage, width, height);

	for (it = m_fontDataMap.begin(); it != m_fontDataMap.end(); ++it)
		delete it->second;
	m_fontDataMap.clear();
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
	int nParams;
	CObject *pObj;
	CObject *pParams[10];

	pSource = m_pPDF->CreateInputStream(pContents);
	pDIS = new CDataInputStream(pSource);
	pReader = new CObjReader(pDIS, m_pPDF->GetXref());
	nParams = 0;
	while (!m_bStop && pDIS->Available() > 0)
	{
		pObj = pReader->ReadObj();
		if (pObj == NULL)
			break;
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

IInputStream *CRenderer::ChangeFont(const char *pName)
{
	CDictionary *pFont;
	map<CObject *, CFontData *>::iterator it;
	const char *cstr, *subpath;
	CObject *pObj, *pEncoding, *pFontFile;
	CArray *pDifferences;
	IInputStream *pSource;
	char file[128], name[16];
	int i, n, index;

	pFont = (CDictionary *)GetResource(FONT, pName);
	assert(pFont);

	it = m_fontDataMap.find(pFont);
	if (it != m_fontDataMap.end())
	{
		m_pFontData = it->second;
		if (m_pFontData->m_pFontFile != NULL)
			m_pFontData->m_pFontFile->Seek(0, SEEK_SET);
	}
	else
	{
		m_pFontData = new CFontData();
		m_fontDataMap.insert(make_pair(pFont, m_pFontData));

		pObj = m_pPDF->GetObject(pFont->GetValue("Subtype"));
		cstr = ((CName *)pObj)->GetValue();
		m_pFontData->m_bSimpleFont = strcmp(cstr, "Type1") == 0 || strcmp(cstr, "MMType1") == 0 || strcmp(cstr, "TrueType") == 0 || strcmp(cstr, "Type3") == 0;
		pObj = m_pPDF->GetObject(pFont->GetValue("ToUnicode"));
		if (pObj != NULL)
		{
			pSource = m_pPDF->CreateInputStream((CStream *)pObj);
			m_pFontData->m_pCMap = ReadCMap(pSource);
			delete pSource;
		}
		else
		{
			pEncoding = m_pPDF->GetObject(pFont->GetValue("Encoding"));
			if (pEncoding != NULL)
				if (m_pFontData->m_bSimpleFont)
				{
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
								m_pFontData->m_pDifferences[index++] = ((CName *)pObj)->GetValue();
						}
						pEncoding = m_pPDF->GetObject(((CDictionary *)pEncoding)->GetValue("BaseEncoding"));
					}
					if (pEncoding != NULL)
					{
						cstr = ((CName *)pEncoding)->GetValue();
						if (strcmp(cstr, "WinAnsiEncoding") == 0)
							m_pFontData->m_pCodeToName = winAnsiEncoding;
						else if (strcmp(cstr, "StandardEncoding") == 0)
							m_pFontData->m_pCodeToName = standardEncoding;
						else if (strcmp(cstr, "MacRomanEncoding") == 0)
							m_pFontData->m_pCodeToName = macRomanEncoding;
						else if (strcmp(cstr, "MacExpertEncoding") == 0)
							m_pFontData->m_pCodeToName = macExpertEncoding;
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
					m_pFontData->m_pCMap = ReadCMap(pSource);
					delete pSource;
					sprintf(file, "/usr/share/poppler/cMap/%s/%s-UCS2", name, name);
					pSource = new CFileInputStream(file);
					m_pFontData->m_pCMap->Concat(ReadCMap(pSource));
					delete pSource;
				}
		}

		pObj = m_pPDF->GetObject(pFont->GetValue("FontDescriptor"));
		if (pObj == NULL)
		{
			pObj = m_pPDF->GetObject(pFont->GetValue("DescendantFonts"));
			if (pObj != NULL)
			{
				pObj = m_pPDF->GetObject(((CArray *)pObj)->GetValue(0));
				if (pObj != NULL)
					pObj = m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("FontDescriptor"));
			}
		}

		if (pObj != NULL)
		{
			pFontFile = m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("FontFile"));
			if (pFontFile == NULL)
				pFontFile = m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("FontFile2"));
			if (pFontFile == NULL)
				pFontFile = m_pPDF->GetObject(((CDictionary *)pObj)->GetValue("FontFile3"));
			if (pFontFile != NULL)
				m_pFontData->m_pFontFile = m_pPDF->CreateInputStream((CStream *)pFontFile);
		}
	}

	return m_pFontData->m_pFontFile;
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
	if (m_pFontData->m_pCMap != NULL)
	{
		if (m_pFontData->m_bSimpleFont)
			while (ustr < (const unsigned char *)cstr + nLen)
			{
				wstr[i] = m_pFontData->m_pCMap->Get(*ustr++);
				if (wstr[i] == 0x00A0)  //WORKAROUND: nbsp?
					wstr[i] = ' ';
				++i;
			}
		else
			while (ustr < (const unsigned char *)cstr + nLen)
			{
				wstr[i] = m_pFontData->m_pCMap->Get((*ustr << 8) | *(ustr + 1));
				if (wstr[i] == 0x00A0)  //WORKAROUND: nbsp?
					wstr[i] = ' ';
				++i;
				ustr += 2;
			}
	}
	else
		while (ustr < (const unsigned char *)cstr + nLen)
		{
			name = m_pFontData->m_pDifferences[*ustr];
			if (name == NULL)
				name = m_pFontData->m_pCodeToName[*ustr];
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
