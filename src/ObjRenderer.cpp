#include "ObjRenderer.h"
#include "Object.h"
#include <stdio.h>
#include <string.h>

ObjRenderer::ObjRenderer(PDF *pPDF) : Renderer(pPDF), m_beginText(false)
{
}

void ObjRenderer::RenderOperator(Operator *pOp, Object **pParams, int nParams)
{
	int i;

	for (i = 0; i < nParams; i++)
	{
		if (m_beginText && pParams[i]->GetType() == Object::OBJ_STRING)
			RenderText((String *)pParams[i]);
		else
			Object::Print(pParams[i]);
		putchar(' ');
	}
	Object::Print(pOp);
	putchar('\n');

	if (strcmp(pOp->GetValue(), "Tf") == 0)
		ChangeFont(((Name *)pParams[0])->GetValue());
	else if (strcmp(pOp->GetValue(), "BT") == 0)
		m_beginText = true;
	else if (strcmp(pOp->GetValue(), "ET") == 0)
		m_beginText = false;
}

void ObjRenderer::RenderString(const char *str)
{
	printf("\"%s\"", str);
}
