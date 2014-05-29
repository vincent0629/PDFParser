#ifndef _OBJRENDERER_H_
#define _OBJRENDERER_H_

#include "Renderer.h"

class CObjRenderer : public CRenderer
{
public:
	CObjRenderer(CPDF *pPDF);

protected:
	void RenderOperator(COperator *pOp, CObject **pParams, int nParams);
	void RenderString(const char *str);

private:
	bool m_beginText;
};

#endif
