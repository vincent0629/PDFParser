#ifndef _TEXTRENDERER_H_
#define _TEXTRENDERER_H_

#include "Renderer.h"

class TextRenderer : public Renderer
{
public:
	TextRenderer(PDF *pPDF);

protected:
	void RenderOperator(Operator *pOp, Object **pParams, int nParams);
	void RenderCharCodes(const uint16_t *codes, int num);
};

#endif
