#ifndef _FONTDATA_H_
#define _FONTDATA_H_

#include <wchar.h>

class CMap;
class Dictionary;
class InputStream;
class PDF;

class FontData
{
public:
	FontData(PDF *pPDF, const Dictionary *pFont);
	~FontData();
	int CharCodesToGlyphs(const char *charCodes, int nLen, int *glyphs);
	int CharCodesToUnicodes(const char *charCodes, int nLen, wchar_t *unicodes);
	InputStream *GetFontFile();

private:
	PDF *m_pPDF;
	const Dictionary *m_pFont;
	bool m_bSimple;
	bool m_bEncoding;
	CMap *m_pCMap;
	bool m_bToUnicode;
	CMap *m_pToUnicode;
	const char **m_pCodeToName;
	const char *m_pDifferences[256];
	bool m_bFontFile;
	InputStream *m_pFontFile;
	int m_nFirstChar, m_nLastChar;
};

#endif
