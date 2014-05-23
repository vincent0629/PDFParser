#include "TextRenderer.h"
#include "Object.h"
#include <stdio.h>
#include <string.h>

CTextRenderer::CTextRenderer(CPDF *pPDF) : CRenderer(pPDF)
{
}

void CTextRenderer::RenderOperator(COperator *pOp, CObject **pParams, int nParams)
{
	const char *cstr;
	int i, n, index;
	CObject *pObj;

	cstr = ((COperator *)pOp)->GetValue();
	if (strcmp(cstr, "Tf") == 0)
		ChangeFont(((CName *)pParams[0])->GetValue());
	else if (strcmp(cstr, "TD") == 0 || strcmp(cstr, "T*") == 0)
		putchar('\n');
	else if (strcmp(cstr, "Tj") == 0)
		RenderText((CString *)pParams[0]);
	else if (strcmp(cstr, "'") == 0)
	{
		putchar('\n');
		RenderText((CString *)pParams[0]);
	}
	else if (strcmp(cstr, "\"") == 0)
	{
		putchar('\n');
		RenderText((CString *)pParams[2]);
	}
	else if (strcmp(cstr, "TJ") == 0)
	{
		n = ((CArray *)pParams[0])->GetSize();
		for (i = 0; i < n; i++)
		{
			pObj = ((CArray *)pParams[0])->GetValue(i);
			if (pObj->GetType() == CObject::OBJ_STRING)
				RenderText((CString *)pObj);
		}
	}
}

void CTextRenderer::RenderString(const char *str)
{
	printf("%s\n", str);
}
