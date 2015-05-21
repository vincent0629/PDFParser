#include "TextRenderer.h"
#include "Object.h"
#include <stdio.h>
#include <string.h>

TextRenderer::TextRenderer(PDF *pPDF) : Renderer(pPDF)
{
}

void TextRenderer::RenderOperator(Operator *pOp, Object **pParams, int nParams)
{
	const char *cstr;
	int i, n, index;
	Object *pObj;

	cstr = ((Operator *)pOp)->GetValue();
	if (strcmp(cstr, "Tf") == 0)
		ChangeFont(((Name *)pParams[0])->GetValue());
	else if (strcmp(cstr, "TD") == 0 || strcmp(cstr, "T*") == 0)
		putchar('\n');
	else if (strcmp(cstr, "Tj") == 0)
		RenderText((String *)pParams[0]);
	else if (strcmp(cstr, "'") == 0)
	{
		putchar('\n');
		RenderText((String *)pParams[0]);
	}
	else if (strcmp(cstr, "\"") == 0)
	{
		putchar('\n');
		RenderText((String *)pParams[2]);
	}
	else if (strcmp(cstr, "TJ") == 0)
	{
		n = ((Array *)pParams[0])->GetSize();
		for (i = 0; i < n; i++)
		{
			pObj = ((Array *)pParams[0])->GetValue(i);
			if (pObj->GetType() == Object::OBJ_STRING)
				RenderText((String *)pObj);
		}
	}
}

void TextRenderer::RenderString(const char *str)
{
	printf("%s\n", str);
}
