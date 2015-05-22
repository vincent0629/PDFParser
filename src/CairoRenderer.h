#ifndef _CAIRORENDERER_H_
#define _CAIRORENDERER_H_

#include "Renderer.h"

struct _cairo;
struct _cairo_matrix;
struct _cairo_surface;
struct _cairo_font_face;

class CairoRenderer : public Renderer
{
public:
	CairoRenderer(PDF *pPDF);

protected:
	void RenderPage(Dictionary *pPage, double dWidth, double dHeight);
	void RenderOperator(Operator *pOp, Object **pParams, int nParams);
	void RenderGlyphs(const uint16_t *glyphs, int num);

private:
	_cairo *m_pCairo;
	_cairo_matrix *m_pFontMatrix;
	double m_pStrokeColor[3];
	int m_nTextMode;
	double m_dTextLead;
	double m_dFontSize;
	_cairo_font_face *m_cairo_face;

	void ConvertNumeric(Object **pParams, int nParams, double *v);
	void SetGraphicsState(const char *pName);
	void SetDash(Object *pArray, Object *pPhase);
	void SetIntent(const char *pName);
	void Stroke(void);
	_cairo_surface *CreateImageSurface(Stream *pStream, int nWidth, int nHeight);
	void SetFontFace(InputStream *pStream);
};

#endif
