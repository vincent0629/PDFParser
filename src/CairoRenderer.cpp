#include "CairoRenderer.h"
#include "InputStream.h"
#include "Object.h"
#include "PDF.h"
#include "FreeType.h"
#include <stdio.h>
#include <string.h>
#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include <assert.h>

//#define DEBUG_OPERATOR
#ifdef DEBUG_OPERATOR
#define NOT_IMPLEMENTED PrintNotImplementedOperator(pOp, pParams, nParams)
#else
#define NOT_IMPLEMENTED
#endif

#ifdef DEBUG_OPERATOR
static void PrintNotImplementedOperator(Operator *pOp, Object **pParams, int nParams)
{
	printf("Not implemented: ");
	for (int i = 0; i < nParams; ++i)
	{
		Object::Print(pParams[i]);
		printf(" ");
	}
	Object::Print(pOp);
	printf("\n");
}
#endif

CairoRenderer::CairoRenderer(PDF *pPDF) : Renderer(pPDF)
{
}

void CairoRenderer::RenderPage(Dictionary *pPage, double dWidth, double dHeight)
{
	Array *pMediaBox;
	cairo_surface_t *pSurface;
	char str[32];

	pSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dWidth, dHeight);
	m_pCairo = cairo_create(pSurface);
	cairo_set_source_rgba(m_pCairo, 1.0, 1.0, 1.0, 1.0);
	cairo_paint(m_pCairo);
	m_pStrokeColor[0] = 0.0;
	m_pStrokeColor[1] = 0.0;
	m_pStrokeColor[2] = 0.0;
	cairo_set_source_rgba(m_pCairo, 0.0, 0.0, 0.0, 1.0);
	cairo_set_line_width(m_pCairo, 0.5);
	cairo_set_miter_limit(m_pCairo, 10.0);
	m_nTextMode = 0;
	m_dTextLead = 0.0;
	m_pFontMatrix = new cairo_matrix_t;

	cairo_translate(m_pCairo, 0, dHeight);
	cairo_scale(m_pCairo, 1.0, -1.0);

	Renderer::RenderPage(pPage, dWidth, dHeight);
	sprintf(str, "%d.png", m_nPage);
	cairo_surface_write_to_png(pSurface, str);

	delete m_pFontMatrix;
	cairo_destroy(m_pCairo);
	cairo_surface_destroy(pSurface);
}

void CairoRenderer::RenderOperator(Operator *pOp, Object **pParams, int nParams)
{
	const char *cstr;
	double x, y;
	double v[6];
	int i, n;
	Object *pObj;
	Stream *pStream;
	Dictionary *pDict;
	cairo_matrix_t matrix;
	int nWidth, nHeight;
	cairo_surface_t *pSurface;

	cstr = pOp->GetValue();

	if (strchr("fFbBW", *cstr) != NULL)
		if (cstr[1] == '\0')
			cairo_set_fill_rule(m_pCairo, CAIRO_FILL_RULE_WINDING);
		else if (cstr[1] == '*')
			cairo_set_fill_rule(m_pCairo, CAIRO_FILL_RULE_EVEN_ODD);

	if (*cstr == 'b')  //close, fill, and stroke
	{
		cairo_close_path(m_pCairo);
		cairo_fill(m_pCairo);
		Stroke();
	}
	else if (strcmp(cstr, "B") == 0 || strcmp(cstr, "B*") == 0)  //fill and stroke
	{
		cairo_fill(m_pCairo);
		Stroke();
	}
	else if (strcmp(cstr, "BDC") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "BI") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "BMC") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "BT") == 0)
	{
		cairo_save(m_pCairo);
		cairo_move_to(m_pCairo, 0.0, 0.0);
		m_cairo_face = NULL;
	}
	else if (strcmp(cstr, "BX") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "c") == 0)  //curve to
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_curve_to(m_pCairo, v[0], v[1], v[2], v[3], v[4], v[5]);
	}
	else if (strcmp(cstr, "cm") == 0)  //concat
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_matrix_init(&matrix, v[0], v[1], v[2], v[3], v[4], v[5]);
		cairo_transform(m_pCairo, &matrix);
	}
	else if (strcmp(cstr, "CS") == 0)  //color space
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "cs") == 0)  //color space
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "d") == 0)  //line dash
		SetDash(pParams[0], pParams[1]);
	else if (strcmp(cstr, "d0") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "d1") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "Do") == 0)
	{
		pStream = (Stream *)GetResource(XOBJECT, ((Name *)pParams[0])->GetValue());
		pDict = pStream->GetDictionary();
		pObj = pDict->GetValue("Subtype");
		if (strcmp(((Name *)pObj)->GetValue(), "Image") == 0)
		{
			nWidth = ((Numeric *)pDict->GetValue("Width"))->GetValue();
			nHeight = ((Numeric *)pDict->GetValue("Height"))->GetValue();
			cairo_matrix_init(&matrix, 1.0 / nWidth, 0.0, 0.0, -1.0 / nHeight, 0.0, 1.0);
			cairo_save(m_pCairo);
			cairo_transform(m_pCairo, &matrix);
			cairo_rectangle(m_pCairo, 0.0, 0.0, nWidth, nHeight);
			cairo_clip(m_pCairo);
			pSurface = CreateImageSurface(pStream, nWidth, nHeight);
			cairo_set_source_surface(m_pCairo, pSurface, 0.0, 0.0);
			cairo_paint(m_pCairo);
			cairo_surface_destroy(pSurface);
			cairo_restore(m_pCairo);
		}
	}
	else if (strcmp(cstr, "DP") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "EI") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "EMC") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "ET") == 0)
	{
		cairo_restore(m_pCairo);
		if (m_cairo_face)
		{
			cairo_set_font_face(m_pCairo, NULL);
			cairo_font_face_destroy(m_cairo_face);
		}
	}
	else if (strcmp(cstr, "EX") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (*cstr == 'f' || *cstr == 'F')  //fill
		cairo_fill(m_pCairo);
	else if (strcmp(cstr, "G") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		m_pStrokeColor[0] = v[0];
		m_pStrokeColor[1] = v[0];
		m_pStrokeColor[2] = v[0];
	}
	else if (strcmp(cstr, "g") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_set_source_rgba(m_pCairo, v[0], v[0], v[0], 1.0);
	}
	else if (strcmp(cstr, "gs") == 0)  //set graphics state
		SetGraphicsState(((Name *)pParams[0])->GetValue());
	else if (strcmp(cstr, "h") == 0)  //close subpath
		cairo_close_path(m_pCairo);
	else if (strcmp(cstr, "i") == 0)  //flatness tolerance
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_set_tolerance(m_pCairo, v[0]);
	}
	else if (strcmp(cstr, "ID") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "j") == 0)  //line join
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_set_line_join(m_pCairo, (cairo_line_join_t)v[0]);
	}
	else if (strcmp(cstr, "J") == 0)  //line cap
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_set_line_cap(m_pCairo, (cairo_line_cap_t)v[0]);
	}
	else if (strcmp(cstr, "K") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		m_pStrokeColor[0] = (1.0 - v[0]) * (1.0 - v[3]);
		m_pStrokeColor[1] = (1.0 - v[1]) * (1.0 - v[3]);
		m_pStrokeColor[2] = (1.0 - v[2]) * (1.0 - v[3]);
	}
	else if (strcmp(cstr, "k") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_set_source_rgba(m_pCairo, (1.0 - v[0]) * (1.0 - v[3]), (1.0 - v[1]) * (1.0 - v[3]), (1.0 - v[2]) * (1.0 - v[3]), 1.0);
	}
	else if (strcmp(cstr, "l") == 0)  //line to
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_line_to(m_pCairo, v[0], v[1]);
	}
	else if (strcmp(cstr, "m") == 0)  //new sub path
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_move_to(m_pCairo, v[0], v[1]);
	}
	else if (strcmp(cstr, "M") == 0)  //miter limit
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_set_miter_limit(m_pCairo, v[0]);
	}
	else if (strcmp(cstr, "MP") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "n") == 0)  //end path
		cairo_new_path(m_pCairo);
	else if (strcmp(cstr, "q") == 0)  //save graphics state
		cairo_save(m_pCairo);
	else if (strcmp(cstr, "Q") == 0)  //restore graphics state
		cairo_restore(m_pCairo);
	else if (strcmp(cstr, "re") == 0)  //rectangle
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_rectangle(m_pCairo, v[0], v[1], v[2], v[3]);
	}
	else if (strcmp(cstr, "RG") == 0)
	{
		ConvertNumeric(pParams, nParams, m_pStrokeColor);
	}
	else if (strcmp(cstr, "rg") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_set_source_rgba(m_pCairo, v[0], v[1], v[2], 1.0);
	}
	else if (strcmp(cstr, "ri") == 0)  //color rendering intent
		SetIntent(((Name *)pParams[0])->GetValue());
	else if (strcmp(cstr, "s") == 0)  //close and stroke
	{
		cairo_close_path(m_pCairo);
		Stroke();
	}
	else if (strcmp(cstr, "S") == 0)  //stroke
		Stroke();
	else if (strcmp(cstr, "SC") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "sc") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "SCN") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "scn") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "sh") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "T*") == 0)
	{
		cairo_translate(m_pCairo, 0.0, -m_dTextLead);
		cairo_move_to(m_pCairo, 0.0, 0.0);
	}
	else if (strcmp(cstr, "Tc") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "Td") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_translate(m_pCairo, v[0], v[1]);
		cairo_move_to(m_pCairo, 0.0, 0.0);
	}
	else if (strcmp(cstr, "TD") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_translate(m_pCairo, v[0], v[1]);
		cairo_move_to(m_pCairo, 0.0, 0.0);
		m_dTextLead = -v[1];
	}
	else if (strcmp(cstr, "Tf") == 0)
	{
		ChangeFont(((Name *)pParams[0])->GetValue());
		SetFontFace(m_pFontData->m_pFontFile);

		ConvertNumeric(pParams + 1, nParams - 1, v);
		cairo_matrix_init_scale(m_pFontMatrix, v[0], -v[0]);
		cairo_set_font_matrix(m_pCairo, m_pFontMatrix);
	}
	else if (strcmp(cstr, "Tj") == 0)
		RenderString((String *)pParams[0]);
	else if (strcmp(cstr, "TJ") == 0)
	{
		n = ((Array *)pParams[0])->GetSize();
		for (i = 0; i < n; i++)
		{
			pObj = ((Array *)pParams[0])->GetValue(i);
			if (pObj->GetType() == Object::OBJ_STRING)
				RenderString((String *)pObj);
			else if (pObj->GetType() == Object::OBJ_NUMERIC)
				cairo_translate(m_pCairo, -((Numeric *)pObj)->GetValue() / 1000.0, 0.0);
		}
	}
	else if (strcmp(cstr, "TL") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		m_dTextLead = v[0];
	}
	else if (strcmp(cstr, "Tm") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_matrix_init(&matrix, v[0], v[1], v[2], v[3], v[4], v[5]);
		cairo_matrix_multiply(&matrix, m_pFontMatrix, &matrix);
		cairo_set_font_matrix(m_pCairo, &matrix);
		cairo_move_to(m_pCairo, 0.0, 0.0);
	}
	else if (strcmp(cstr, "Tr") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		m_nTextMode = v[0];
	}
	else if (strcmp(cstr, "Ts") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "Tw") == 0)
	{
		NOT_IMPLEMENTED;
	}
	else if (strcmp(cstr, "Tz") == 0)
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_scale(m_pCairo, v[0] / 100.0, 1.0);
	}
	else if (strcmp(cstr, "v") == 0)  //curve to
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_get_current_point(m_pCairo, &x, &y);
		cairo_curve_to(m_pCairo, x, y, v[0], v[1], v[2], v[3]);
	}
	else if (strcmp(cstr, "w") == 0)  //line width
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_set_line_width(m_pCairo, v[0] + 0.5);
	}
	else if (*cstr == 'W')  //clipping path
		cairo_clip(m_pCairo);
	else if (strcmp(cstr, "y") == 0)  //curve to
	{
		ConvertNumeric(pParams, nParams, v);
		cairo_curve_to(m_pCairo, v[0], v[1], v[2], v[3], v[2], v[3]);
	}
	else if (strcmp(cstr, "'") == 0)
	{
		cairo_translate(m_pCairo, 0.0, -m_dTextLead);
		cairo_move_to(m_pCairo, 0.0, 0.0);
		RenderString((String *)pParams[0]);
	}
	else if (strcmp(cstr, "\"") == 0)
	{
		cairo_translate(m_pCairo, 0.0, -m_dTextLead);
		cairo_move_to(m_pCairo, 0.0, 0.0);
		RenderString((String *)pParams[2]);
	}
	else
	{
		assert(false);
	}
}

void CairoRenderer::RenderGlyphs(const uint16_t *glyphs, int num)
{
	cairo_glyph_t *cg;
	int i;

	cg = new cairo_glyph_t[num];
	for (i = 0; i < num; ++i)
	{
		cg[i].index = glyphs[i];
		cg[i].x = 0;
		cg[i].y = 0;
	}
	cairo_show_glyphs(m_pCairo, cg, num);
	delete[] cg;
}

void CairoRenderer::ConvertNumeric(Object **pParams, int nParams, double *v)
{
	int i;

	for (i = 0; i < nParams; i++)
		if (pParams[i]->GetType() == Object::OBJ_NUMERIC)
			v[i] = ((Numeric *)pParams[i])->GetValue();
}

void CairoRenderer::SetGraphicsState(const char *pName)
{
	Dictionary *pGState;
	int i, n;
	Object *pObj;
	double v;

	pGState = (Dictionary *)GetResource(EXTGSTATE, pName);
	n = pGState->GetSize();
	for (i = 0; i < n; i++)
	{
		pName = pGState->GetName(i);
		pObj = pGState->GetValue(pName);
		if (pObj->GetType() == Object::OBJ_NUMERIC)
			v = ((Numeric *)pObj)->GetValue();
		if (strcmp(pName, "Type") == 0)
			assert(strcmp(((String *)pObj)->GetValue(), "ExtGState") == 0);
		else if (strcmp(pName, "LW") == 0)
			cairo_set_line_width(m_pCairo, v + 0.5);
		else if (strcmp(pName, "LC") == 0)
			cairo_set_line_cap(m_pCairo, (cairo_line_cap_t)v);
		else if (strcmp(pName, "LJ") == 0)
			cairo_set_line_join(m_pCairo, (cairo_line_join_t)v);
		else if (strcmp(pName, "ML") == 0)
			cairo_set_miter_limit(m_pCairo, v);
		else if (strcmp(pName, "D") == 0)
			SetDash(((Array *)pObj)->GetValue(0), ((Array *)pObj)->GetValue(1));
		else if (strcmp(pName, "RI") == 0)
			SetIntent(((Name *)pObj)->GetValue());
		else
		{
			//not implemented
#ifdef DEBUG_OPERATOR
			printf("Not implemented: %s:%d %s\n", __func__, __LINE__, pName);
#endif
		}
	}
}

void CairoRenderer::SetDash(Object *pArray, Object *pPhase)
{
	Array *array;
	int i, n;
	double *dash;

	array = (Array *)pArray;
	n = array->GetSize();
	dash = new double[n];
	for (i = 0; i < n; i++)
		dash[i] = ((Numeric *)array->GetValue(i))->GetValue();
	cairo_set_dash(m_pCairo, dash, n, ((Numeric *)pPhase)->GetValue());
	delete[] dash;
}

void CairoRenderer::SetIntent(const char *pName)
{
	//not implemented
#ifdef DEBUG_OPERATOR
	printf("Not Implemented: %s:%d\n", __func__, __LINE__);
#endif
}

void CairoRenderer::Stroke(void)
{
	double r, g, b, a;

	cairo_pattern_get_rgba(cairo_get_source(m_pCairo), &r, &g, &b, &a);
	cairo_set_source_rgba(m_pCairo, m_pStrokeColor[0], m_pStrokeColor[1], m_pStrokeColor[2], 1.0);
	cairo_stroke(m_pCairo);
	cairo_set_source_rgba(m_pCairo, r, g, b, 1.0);
}

cairo_surface_t *CairoRenderer::CreateImageSurface(Stream *pStream, int nWidth, int nHeight)
{
	cairo_surface_t *pSurface;
	unsigned char *ptr, pixel[3];
	InputStream *pSource;
	int i;

	pSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nWidth, nHeight);
	ptr = cairo_image_surface_get_data(pSurface);
	pSource = m_pPDF->CreateInputStream(pStream);
	for (i = nWidth * nHeight; i > 0; --i)
	{
		pSource->Read(pixel, 3);
		ptr[0] = pixel[2];
		ptr[1] = pixel[1];
		ptr[2] = pixel[0];
		ptr[3] = 0xFF;
		ptr += 4;
	}
	delete pSource;
	cairo_surface_mark_dirty(pSurface);

	return pSurface;
}
 
void CairoRenderer::SetFontFace(InputStream *pStream)
{
	FT_Face face;
	static cairo_user_data_key_t key;

	if (m_cairo_face)
	{
		cairo_set_font_face(m_pCairo, NULL);
		cairo_font_face_destroy(m_cairo_face);
		m_cairo_face = NULL;
	}

	if (pStream)
	{
		face = FreeType::OpenFace(pStream);  // face will be destroyed by FT_Done_Face
		if (face)
		{
			m_cairo_face = cairo_ft_font_face_create_for_ft_face(face, 0);
			cairo_font_face_set_user_data(m_cairo_face, &key, face, (cairo_destroy_func_t)FT_Done_Face);
		}
	}
	else
		m_cairo_face = cairo_toy_font_face_create("unifont", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

	if (m_cairo_face)
		cairo_set_font_face(m_pCairo, m_cairo_face);
}
