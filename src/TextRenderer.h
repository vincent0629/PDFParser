#ifndef _TEXTRENDERER_H_
#define _TEXTRENDERER_H_

#include "Renderer.h"

class CTextRenderer : public CRenderer
{
public:
	CTextRenderer(CPDF *pPDF);

protected:
	void RenderOperator(COperator *pOp, CObject **pParams, int nParams);
	void RenderString(const char *str);
};

#endif
