#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <map>

class Dictionary;
class FontData;
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
	const Dictionary *m_pResourceRoot;
	FontData *m_pFontData;

	const Object *GetResource(ResourceType nRes, const char *pName);
	virtual void RenderPage(const Dictionary *pPage, double dWidth, double dHeight);
	virtual void RenderContents(const Stream *pContents);
	virtual void RenderOperator(const Operator *pOp, const Object **pParams, int nParams);
	void ChangeFont(const char *pName);
	virtual void RenderString(const String *pString);

private:
	const Dictionary *m_pResources[RES_NUM];
	std::map<const Object *, FontData *> m_fontDataMap;
	bool m_bStop;
};

#endif
