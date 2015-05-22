#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <stdint.h>
#include <map>

class CMap;
class Dictionary;
class FontData;
class InputStream;
class Object;
class Operator;
class PDF;
class Stream;
class String;

class FontData
{
public:
	bool m_bSimpleFont;
	CMap *m_pCMap, *m_pToUnicode;
	const char **m_pCodeToName;
	const char *m_pDifferences[256];
	InputStream *m_pFontFile;

	FontData();
	~FontData();
};

class Renderer
{
public:
	typedef enum
	{
		COLORSPACE,
		FONT,
		EXTGSTATE,
		XOBJECT,
		RES_NUM
	} ResourceType;
	Renderer(PDF *pPDF);
	virtual ~Renderer();
	virtual void Render(int nPage);
	void Stop(void);

protected:
	PDF *m_pPDF;
	int m_nPage;
	Dictionary *m_pResourceRoot;
	FontData *m_pFontData;

	Object *GetResource(ResourceType nRes, const char *pName);
	virtual void RenderPage(Dictionary *pPage, double dWidth, double dHeight);
	virtual void RenderContents(Stream *pContents);
	virtual void RenderOperator(Operator *pOp, Object **pParams, int nParams);
	void ChangeFont(const char *pName);
	void RenderString(String *pString);
	virtual void RenderCharCodes(const uint16_t *codes, int num);
	virtual void RenderGlyphs(const uint16_t *glyphs, int num);

private:
	Dictionary *m_pResources[RES_NUM];
	std::map<Object *, FontData *> m_fontDataMap;
	bool m_bStop;
};

#endif
