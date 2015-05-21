#ifndef _RENDERERFACTORY_H_
#define _RENDERERFACTORY_H_

class PDF;
class Renderer;

class RendererFactory
{
public:
	typedef enum
	{
		OBJECT,
		RAW,
		TEXT,
		CAIRO
	} RendererType;

	static Renderer *Create(RendererType nType, PDF *pPDF);
};

#endif
