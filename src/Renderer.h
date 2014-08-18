#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <map>

class CCMap;
class CDictionary;
class CFontData;
class IInputStream;
class CObject;
class COperator;
class CPDF;
class CStream;
class CString;

class CRenderer
{
public:
	typedef enum
	{
		COLORSPACE,
		FONT,
		EXTGSTATE,
		XOBJECT,
		RES_NUM
	} Resource_t;
	CRenderer(CPDF *pPDF);
	virtual ~CRenderer();
	virtual void Render(int nPage);
	void Stop(void);

protected:
	CPDF *m_pPDF;
	int m_nPage;
	CDictionary *m_pResourceRoot;

	CObject *GetResource(Resource_t nRes, const char *pName);
	virtual void RenderPage(CDictionary *pPage, double dWidth, double dHeight);
	virtual void RenderContents(CStream *pContents);
	virtual void RenderOperator(COperator *pOp, CObject **pParams, int nParams);
	IInputStream *ChangeFont(const char *pName);
	void RenderText(CString *pString);
	virtual void RenderString(const char *str);

private:
	CDictionary *m_pResources[RES_NUM];
	std::map<CObject *, CFontData *> m_fontDataMap;
	CFontData *m_pFontData;
	bool m_bStop;
};

#endif
