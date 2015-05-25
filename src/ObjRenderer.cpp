#include "ObjRenderer.h"
#include "Object.h"
#include <stdio.h>
#include <string.h>

ObjRenderer::ObjRenderer(PDF *pPDF) : Renderer(pPDF), m_beginText(false)
{
}

void ObjRenderer::RenderOperator(const Operator *pOp, const Object **pParams, int nParams)
{
	int i;
	const char *cstr;

	for (i = 0; i < nParams; i++)
	{
		if (m_beginText && pParams[i]->GetType() == Object::OBJ_STRING)
			RenderString((const String *)pParams[i]);
		else
			Object::Print(pParams[i]);
		putchar(' ');
	}
	Object::Print(pOp);
	putchar('\n');

	cstr = pOp->GetValue();
	if (strcmp(cstr, "Tf") == 0)
		ChangeFont(((const Name *)pParams[0])->GetValue());
	else if (strcmp(cstr, "BT") == 0)
		m_beginText = true;
	else if (strcmp(cstr, "ET") == 0)
		m_beginText = false;
}

void ObjRenderer::RenderString(const String *pString)
{
	const unsigned char *codes;
	int i, n;

	codes = (const unsigned char *)pString->GetValue();
	n = pString->GetLength();
	if (n > 0)
		printf("%d", codes[0]);
	for (i = 1; i < n; ++i)
		printf(" %d", codes[i]);
}
