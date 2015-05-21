#ifndef _OBJRENDERER_H_
#define _OBJRENDERER_H_

#include "Renderer.h"

class ObjRenderer : public Renderer
{
public:
	ObjRenderer(PDF *pPDF);

protected:
	void RenderOperator(Operator *pOp, Object **pParams, int nParams);
	void RenderString(const char *str);

private:
	bool m_beginText;
};

#endif
