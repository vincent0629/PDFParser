#include "FontData.h"
#include "CMap.h"
#include "CMapReader.h"
#include "FileInputStream.h"
#include "FontEncoding.h"
#include "InputStream.h"
#include "Object.h"
#include "PDF.h"
#include <stdio.h>
#include <string.h>

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

FontData::FontData(PDF *pPDF, const Dictionary *pFont)
	: m_pPDF(pPDF)
	, m_pFont(pFont)
	, m_bSimple(true)
	, m_bEncoding(false)
	, m_pCMap(NULL)
	, m_bToUnicode(false)
	, m_pToUnicode(NULL)
	, m_pCodeToName(standardEncoding)
	, m_bFontFile(false)
	, m_pFontFile(NULL)
{
	const Object *pObj;
	const char *cstr;

	memset(m_pDifferences, 0, sizeof(m_pDifferences));

	pObj = m_pPDF->GetObject(m_pFont->GetValue("Subtype"));
	cstr = ((const Name *)pObj)->GetValue();
	m_bSimple = strcmp(cstr, "Type1") == 0 || strcmp(cstr, "MMType1") == 0 || strcmp(cstr, "TrueType") == 0 || strcmp(cstr, "Type3") == 0;
}

FontData::~FontData()
{
	delete m_pCMap;
	delete m_pToUnicode;
	delete m_pFontFile;
}

int FontData::CharCodesToGlyphs(const char *charCodes, int nLen, int *glyphs)
{
	int i, n, index;
	const Object *pObj, *pEncoding;
	const Array *pDifferences;
	const char *cstr;
	char name[16], file[128];
	InputStream *pSource;
	const unsigned char *ptr;

	pEncoding = m_pPDF->GetObject(m_pFont->GetValue("Encoding"));
	if (pEncoding != NULL)
		if (m_bSimple)
		{
			if (pEncoding->GetType() == Object::OBJ_DICTIONARY)
			{
				pDifferences = (const Array *)m_pPDF->GetObject(((const Dictionary *)pEncoding)->GetValue("Differences"));
				index = 0;
				n = pDifferences->GetSize();
				for (i = 0; i < n; i++)
				{
					pObj = pDifferences->GetValue(i);
					if (pObj->GetType() == Object::OBJ_NUMERIC)
						index = ((const Numeric *)pObj)->GetValue();
					else
						m_pDifferences[index++] = ((const Name *)pObj)->GetValue();
				}
				pEncoding = m_pPDF->GetObject(((const Dictionary *)pEncoding)->GetValue("BaseEncoding"));
			}
			if (pEncoding != NULL)
			{
				cstr = ((const Name *)pEncoding)->GetValue();
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
			cstr = ((const Name *)pEncoding)->GetValue();
			pObj = m_pPDF->GetObject(m_pFont->GetValue("DescendantFonts"));
			pObj = m_pPDF->GetObject(((const Array *)pObj)->GetValue(0));
			pObj = m_pPDF->GetObject(((const Dictionary *)pObj)->GetValue("CIDSystemInfo"));
			strcpy(name, ((const String *)m_pPDF->GetObject(((const Dictionary *)pObj)->GetValue("Registry")))->GetValue());
			strcat(name, "-");
			strcat(name, ((const String *)m_pPDF->GetObject(((const Dictionary *)pObj)->GetValue("Ordering")))->GetValue());
			sprintf(file, "/usr/share/poppler/cMap/%s/%s", strncmp(cstr, "Identity", 8) == 0? "" : name, cstr);
			pSource = new FileInputStream(file);
			m_pCMap = ReadCMap(pSource);
			delete pSource;
			sprintf(file, "/usr/share/poppler/cMap/%s/%s-UCS2", name, name);
			pSource = new FileInputStream(file);
			m_pCMap->Concat(ReadCMap(pSource));
			delete pSource;
		}

	ptr = (const unsigned char *)charCodes;
	if (m_pCMap != NULL)
	{
		for (i = 0; i < nLen; ++i)
			glyphs[i] = m_pCMap->Get(ptr[i]);
	}
	else
	{
	}

	return nLen;
}

int FontData::CharCodesToUnicodes(const char *charCodes, int nLen, wchar_t *unicodes)
{
	int i;
	const Object *pObj;
	InputStream *pSource;
	const unsigned char *ptr;

	if (!m_bToUnicode)
	{
		m_bToUnicode = true;
		pObj = m_pPDF->GetObject(m_pFont->GetValue("ToUnicode"));
		if (pObj != NULL)
		{
			pSource = m_pPDF->CreateInputStream((const Stream *)pObj);
			m_pToUnicode = ReadCMap(pSource);
			delete pSource;
		}
	}

	ptr = (const unsigned char *)charCodes;
	if (m_bSimple)
	{
		if (m_pToUnicode)
			for (i = 0; i < nLen; ++i)
				unicodes[i] = m_pToUnicode->Get(ptr[i]);
		else
			for (i = 0; i < nLen; ++i)
				unicodes[i] = ptr[i];
	}
	else
	{
		nLen /= 2;
		if (m_pToUnicode)
			for (i = 0; i < nLen; ++i)
				unicodes[i] = m_pToUnicode->Get((ptr[i * 2] << 8) | ptr[i * 2 + 1]);
		else
			for (i = 0; i < nLen; ++i)
				unicodes[i] = (ptr[i * 2] << 8) | ptr[i * 2 + 1];
	}

	return nLen;
}

InputStream *FontData::GetFontFile()
{
	const Object *pObj, *pFontFile;

	if (m_bFontFile)
		return m_pFontFile;

	m_bFontFile = true;
	pObj = m_pPDF->GetObject(m_pFont->GetValue("FontDescriptor"));
	if (pObj == NULL)
	{
		pObj = m_pPDF->GetObject(m_pFont->GetValue("DescendantFonts"));
		if (pObj != NULL)
		{
			pObj = m_pPDF->GetObject(((const Array *)pObj)->GetValue(0));
			if (pObj != NULL)
				pObj = m_pPDF->GetObject(((const Dictionary *)pObj)->GetValue("FontDescriptor"));
		}
	}

	if (pObj != NULL)  // FontDescriptor
	{
		pFontFile = m_pPDF->GetObject(((const Dictionary *)pObj)->GetValue("FontFile"));
		if (pFontFile == NULL)
			pFontFile = m_pPDF->GetObject(((const Dictionary *)pObj)->GetValue("FontFile2"));
		if (pFontFile == NULL)
			pFontFile = m_pPDF->GetObject(((const Dictionary *)pObj)->GetValue("FontFile3"));
		if (pFontFile != NULL)
			m_pFontFile = m_pPDF->CreateInputStream((const Stream *)pFontFile);
	}

	return m_pFontFile;
}
