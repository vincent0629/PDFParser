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
	const char *cstr;

	for (i = 0; i < nParams; i++)
	{
		if (m_beginText && pParams[i]->GetType() == Object::OBJ_STRING)
			RenderString((String *)pParams[i]);
		else
			Object::Print(pParams[i]);
		putchar(' ');
	}
	Object::Print(pOp);
	putchar('\n');

	cstr = pOp->GetValue();
	if (strcmp(cstr, "Tf") == 0)
		ChangeFont(((Name *)pParams[0])->GetValue());
	else if (strcmp(cstr, "BT") == 0)
		m_beginText = true;
	else if (strcmp(cstr, "ET") == 0)
		m_beginText = false;
}

void ObjRenderer::RenderCharCodes(const uint16_t *codes, int num)
{
	int i;

	if (num > 0)
		printf("%d", codes[0]);
	for (i = 1; i < num; ++i)
		printf(" %d", codes[i]);
}
