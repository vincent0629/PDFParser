#ifndef _TEXTRENDERER_H_
#define _TEXTRENDERER_H_

#include "Renderer.h"

class TextRenderer : public Renderer
{
public:
	TextRenderer(PDF *pPDF);

protected:
	void RenderOperator(const Operator *pOp, const Object **pParams, int nParams);
	void RenderString(const String *pString);
};

#endif
