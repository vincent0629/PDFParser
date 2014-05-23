#include "RendererFactory.h"
#include "CairoRenderer.h"
#include "ObjRenderer.h"
#include "RawRenderer.h"
#include "TextRenderer.h"

CRenderer *CRendererFactory::Create(Renderer_t nType, CPDF *pPDF)
{
	CRenderer *pRenderer;

	switch (nType)
	{
		case OBJECT:
			pRenderer = new CObjRenderer(pPDF);
			break;
		case RAW:
			pRenderer = new CRawRenderer(pPDF);
			break;
		case TEXT:
			pRenderer = new CTextRenderer(pPDF);
			break;
		case CAIRO:
			pRenderer = new CCairoRenderer(pPDF);
			break;
	}
	return pRenderer;
}
