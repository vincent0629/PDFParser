#ifndef _RENDERERFACTORY_H_
#define _RENDERERFACTORY_H_

class CPDF;
class CRenderer;

class CRendererFactory
{
public:
	typedef enum
	{
		OBJECT,
		RAW,
		TEXT,
		CAIRO
	} Renderer_t;

	static CRenderer *Create(Renderer_t nType, CPDF *pPDF);
};

#endif
