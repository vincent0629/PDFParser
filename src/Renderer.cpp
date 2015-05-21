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

static CMap *ReadCMap(InputStream *pSource)
{
	CMap *pCMap;
	CMapReader *pReader;

	pCMap = new CMap();
	pReader = new CMapReader(pCMap);
	pReader->Read(pSource);
	delete pReader;
	return pCMap;
}

class FontData
{
public:
	bool m_bSimpleFont;
	CMap *m_pCMap;
	const char **m_pCodeToName;
	const char *m_pDifferences[256];
	InputStream *m_pFontFile;

	FontData();
	~FontData();
};

FontData::FontData()
	: m_bSimpleFont(true)
	, m_pCMap(NULL)
	, m_pCodeToName(standardEncoding)
	, m_pFontFile(NULL)
{
	memset(m_pDifferences, 0, sizeof(m_pDifferences));
}

FontData::~FontData()
{
	delete m_pCMap;
	delete m_pFontFile;
}

Renderer::Renderer(PDF *pPDF)
{
	m_pPDF = pPDF;
}

Renderer::~Renderer()
{
}

void Renderer::Render(int nPage)
{
	Dictionary *pPage;
	Array *pMediaBox;
	double width, height;
	map<Object *, FontData *>::iterator it;

	pPage = m_pPDF->GetPage(nPage);
	if (pPage == NULL)
		return;

	m_nPage = nPage;
	m_pResourceRoot = (Dictionary *)m_pPDF->GetObject(pPage->GetValue("Resources"));
	memset(m_pResources, 0, sizeof(Dictionary *) * RES_NUM);
	m_bStop = false;

	pMediaBox = (Array *)pPage->GetValue("MediaBox");
	if (!pMediaBox)
	{
		width = 600;
		height = 800;
	}
	else
	{
		width = ((Numeric *)pMediaBox->GetValue(2))->GetValue();
		height = ((Numeric *)pMediaBox->GetValue(3))->GetValue();
	}

	RenderPage(pPage, width, height);

	for (it = m_fontDataMap.begin(); it != m_fontDataMap.end(); ++it)
		delete it->second;
	m_fontDataMap.clear();
	delete pPage;
}

void Renderer::Stop(void)
{
	m_bStop = true;
}

Object *Renderer::GetResource(ResourceType nRes, const char *pName)
{
	const char *pNames[] = {"ColorSpace", "Font", "ExtGState", "XObject"};

	if (m_pResources[nRes] == NULL && m_pResourceRoot != NULL)
		m_pResources[nRes] = (Dictionary *)m_pPDF->GetObject(m_pResourceRoot->GetValue(pNames[nRes]));
	return m_pResources[nRes] == NULL? NULL : m_pPDF->GetObject(m_pResources[nRes]->GetValue(pName));
}

void Renderer::RenderPage(Dictionary *pPage, double dWidth, double dHeight)
{
	Object *pObj;
	int i, n;

	pObj = m_pPDF->GetObject(pPage->GetValue("Contents"));
	if (pObj->GetType() == Object::OBJ_STREAM)
		RenderContents((Stream *)pObj);
	else if (pObj->GetType() == Object::OBJ_ARRAY)
	{
		n = ((Array *)pObj)->GetSize();
		for (i = 0; i < n && !m_bStop; i++)
			RenderContents((Stream *)m_pPDF->GetObject(((Array *)pObj)->GetValue(i)));
	}
}

void Renderer::RenderContents(Stream *pContents)
{
	InputStream *pSource;
	DataInputStream *pDIS;
	ObjReader *pReader;
	int nParams;
	Object *pObj;
	Object *pParams[10];

	pSource = m_pPDF->CreateInputStream(pContents);
	pDIS = new DataInputStream(pSource);
	pReader = new ObjReader(pDIS, m_pPDF->GetXref());
	nParams = 0;
	while (!m_bStop && pDIS->Available() > 0)
	{
		pObj = pReader->ReadObj();
		if (pObj == NULL)
			break;
		if (pObj->GetType() == Object::OBJ_OPERATOR)
		{
			RenderOperator((Operator *)pObj, pParams, nParams);
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

void Renderer::RenderOperator(Operator *pOp, Object **pParams, int nParams)
{
}

InputStream *Renderer::ChangeFont(const char *pName)
{
	Dictionary *pFont;
	map<Object *, FontData *>::iterator it;
	const char *cstr, *subpath;
	Object *pObj, *pEncoding, *pFontFile;
	Array *pDifferences;
	InputStream *pSource;
	char file[128], name[16];
	int i, n, index;

	pFont = (Dictionary *)GetResource(FONT, pName);
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
		m_pFontData = new FontData();
		m_fontDataMap.insert(make_pair(pFont, m_pFontData));

		pObj = m_pPDF->GetObject(pFont->GetValue("Subtype"));
		cstr = ((Name *)pObj)->GetValue();
		m_pFontData->m_bSimpleFont = strcmp(cstr, "Type1") == 0 || strcmp(cstr, "MMType1") == 0 || strcmp(cstr, "TrueType") == 0 || strcmp(cstr, "Type3") == 0;
		pObj = m_pPDF->GetObject(pFont->GetValue("ToUnicode"));
		if (pObj != NULL)
		{
			pSource = m_pPDF->CreateInputStream((Stream *)pObj);
			m_pFontData->m_pCMap = ReadCMap(pSource);
			delete pSource;
		}
		else
		{
			pEncoding = m_pPDF->GetObject(pFont->GetValue("Encoding"));
			if (pEncoding != NULL)
				if (m_pFontData->m_bSimpleFont)
				{
					if (pEncoding->GetType() == Object::OBJ_DICTIONARY)
					{
						pDifferences = (Array *)m_pPDF->GetObject(((Dictionary *)pEncoding)->GetValue("Differences"));
						index = 0;
						n = pDifferences->GetSize();
						for (i = 0; i < n; i++)
						{
							pObj = pDifferences->GetValue(i);
							if (pObj->GetType() == Object::OBJ_NUMERIC)
								index = ((Numeric *)pObj)->GetValue();
							else
								m_pFontData->m_pDifferences[index++] = ((Name *)pObj)->GetValue();
						}
						pEncoding = m_pPDF->GetObject(((Dictionary *)pEncoding)->GetValue("BaseEncoding"));
					}
					if (pEncoding != NULL)
					{
						cstr = ((Name *)pEncoding)->GetValue();
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
					cstr = ((Name *)pEncoding)->GetValue();
					pObj = m_pPDF->GetObject(pFont->GetValue("DescendantFonts"));
					pObj = m_pPDF->GetObject(((Array *)pObj)->GetValue(0));
					pObj = m_pPDF->GetObject(((Dictionary *)pObj)->GetValue("CIDSystemInfo"));
					strcpy(name, ((String *)m_pPDF->GetObject(((Dictionary *)pObj)->GetValue("Registry")))->GetValue());
					strcat(name, "-");
					strcat(name, ((String *)m_pPDF->GetObject(((Dictionary *)pObj)->GetValue("Ordering")))->GetValue());
					sprintf(file, "/usr/share/poppler/cMap/%s/%s", strncmp(cstr, "Identity", 8) == 0? "" : name, cstr);
					pSource = new FileInputStream(file);
					m_pFontData->m_pCMap = ReadCMap(pSource);
					delete pSource;
					sprintf(file, "/usr/share/poppler/cMap/%s/%s-UCS2", name, name);
					pSource = new FileInputStream(file);
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
				pObj = m_pPDF->GetObject(((Array *)pObj)->GetValue(0));
				if (pObj != NULL)
					pObj = m_pPDF->GetObject(((Dictionary *)pObj)->GetValue("FontDescriptor"));
			}
		}

		if (pObj != NULL)
		{
			pFontFile = m_pPDF->GetObject(((Dictionary *)pObj)->GetValue("FontFile"));
			if (pFontFile == NULL)
				pFontFile = m_pPDF->GetObject(((Dictionary *)pObj)->GetValue("FontFile2"));
			if (pFontFile == NULL)
				pFontFile = m_pPDF->GetObject(((Dictionary *)pObj)->GetValue("FontFile3"));
			if (pFontFile != NULL)
				m_pFontData->m_pFontFile = m_pPDF->CreateInputStream((Stream *)pFontFile);
		}
	}

	return m_pFontData->m_pFontFile;
}

void Renderer::RenderText(String *pString)
{
	const char *cstr, *name;
	const unsigned char *ustr;
	wchar_t *wstr;
	char *str;
	int i, nLen;
	NameToUnicode *pNameToUnicode;

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

void Renderer::RenderString(const char *str)
{
}
