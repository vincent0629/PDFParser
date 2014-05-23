#ifndef _RAWRENDERER_H_
#define _RAWRENDERER_H_

#include "Renderer.h"

class CRawRenderer : public CRenderer
{
public:
	CRawRenderer(CPDF *pPDF);

protected:
	void RenderContents(CStream *pContents);
};

#endif
