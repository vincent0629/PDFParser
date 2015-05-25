#ifndef _OBJRENDERER_H_
#define _OBJRENDERER_H_

#include "Renderer.h"

class ObjRenderer : public Renderer
{
public:
	ObjRenderer(PDF *pPDF);

protected:
	void RenderOperator(const Operator *pOp, const Object **pParams, int nParams);
	void RenderString(const String *pString);

private:
	bool m_beginText;
};

#endif
