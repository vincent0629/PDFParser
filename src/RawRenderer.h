#ifndef _RAWRENDERER_H_
#define _RAWRENDERER_H_

#include "Renderer.h"

class RawRenderer : public Renderer
{
public:
	RawRenderer(PDF *pPDF);

protected:
	void RenderContents(Stream *pContents);
};

#endif
