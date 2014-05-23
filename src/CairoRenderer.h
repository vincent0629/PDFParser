#ifndef _CAIRORENDERER_H_
#define _CAIRORENDERER_H_

#include "Renderer.h"

struct _cairo;
struct _cairo_matrix;

class CCairoRenderer : public CRenderer
{
public:
	CCairoRenderer(CPDF *pPDF);

protected:
	void RenderPage(CDictionary *pPage, double dWidth, double dHeight);
	void RenderOperator(COperator *pOp, CObject **pParams, int nParams);
	void RenderString(const char *str);

private:
	double m_dHeight;
	_cairo *m_pCairo;
	_cairo_matrix *m_pFontMatrix;
	double m_pStrokeColor[3];
	int m_nTextMode;
	double m_dTextLead;
	double m_dFontSize;

	void ConvertNumeric(CObject **pParams, int nParams, double *v);
	void SetGraphicsState(const char *pName);
	void SetDash(CObject *pArray, CObject *pPhase);
	void SetIntent(const char *pName);
	void Stroke(void);
};

#endif
