#ifndef _CAIRORENDERER_H_
#define _CAIRORENDERER_H_

#include "Renderer.h"

class InputStream;

struct _cairo;
struct _cairo_matrix;
struct _cairo_surface;
struct _cairo_font_face;

class CairoRenderer : public Renderer
{
public:
	CairoRenderer(PDF *pPDF);

protected:
	void RenderPage(const Dictionary *pPage, double dWidth, double dHeight);
	void RenderOperator(const Operator *pOp, const Object **pParams, int nParams);
	void RenderString(const String *pString);

private:
	_cairo *m_pCairo;
	_cairo_matrix *m_pFontMatrix;
	double m_pStrokeColor[3];
	int m_nTextMode;
	double m_dTextLead;
	double m_dFontSize;
	_cairo_font_face *m_cairo_face;

	void ConvertNumeric(const Object **pParams, int nParams, double *v);
	void SetGraphicsState(const char *pName);
	void SetDash(const Object *pArray, const Object *pPhase);
	void SetIntent(const char *pName);
	void Stroke(void);
	_cairo_surface *CreateImageSurface(const Stream *pStream, int nWidth, int nHeight);
	void SetFontFace(InputStream *pStream);
};

#endif
