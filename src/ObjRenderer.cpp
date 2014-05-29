#include "ObjRenderer.h"
#include "Object.h"
#include <stdio.h>
#include <string.h>

CObjRenderer::CObjRenderer(CPDF *pPDF) : CRenderer(pPDF), m_beginText(false)
{
}

void CObjRenderer::RenderOperator(COperator *pOp, CObject **pParams, int nParams)
{
	int i;

	for (i = 0; i < nParams; i++)
	{
		if (m_beginText && pParams[i]->GetType() == CObject::OBJ_STRING)
			RenderText((CString *)pParams[i]);
		else
			CObject::Print(pParams[i]);
		putchar(' ');
	}
	CObject::Print(pOp);
	putchar('\n');

	if (strcmp(pOp->GetValue(), "Tf") == 0)
		ChangeFont(((CName *)pParams[0])->GetValue());
	else if (strcmp(pOp->GetValue(), "BT") == 0)
		m_beginText = true;
	else if (strcmp(pOp->GetValue(), "ET") == 0)
		m_beginText = false;
}

void CObjRenderer::RenderString(const char *str)
{
	printf("\"%s\"", str);
}
