#include "RendererFactory.h"
#include "CairoRenderer.h"
#include "ObjRenderer.h"
#include "RawRenderer.h"
#include "TextRenderer.h"

Renderer *RendererFactory::Create(RendererType nType, PDF *pPDF)
{
	Renderer *pRenderer;

	switch (nType)
	{
		case OBJECT:
			pRenderer = new ObjRenderer(pPDF);
			break;
		case RAW:
			pRenderer = new RawRenderer(pPDF);
			break;
		case TEXT:
			pRenderer = new TextRenderer(pPDF);
			break;
		case CAIRO:
			pRenderer = new CairoRenderer(pPDF);
			break;
	}
	return pRenderer;
}
