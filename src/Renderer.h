#ifndef _RENDERER_H_
#define _RENDERER_H_

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

	Object *GetResource(ResourceType nRes, const char *pName);
	virtual void RenderPage(Dictionary *pPage, double dWidth, double dHeight);
	virtual void RenderContents(Stream *pContents);
	virtual void RenderOperator(Operator *pOp, Object **pParams, int nParams);
	InputStream *ChangeFont(const char *pName);
	void RenderText(String *pString);
	virtual void RenderString(const char *str);

private:
	Dictionary *m_pResources[RES_NUM];
	std::map<Object *, FontData *> m_fontDataMap;
	FontData *m_pFontData;
	bool m_bStop;
};

#endif
