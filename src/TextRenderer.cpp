#include "TextRenderer.h"
#include "Object.h"
#include "CMap.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
		RenderString((String *)pParams[0]);
	else if (strcmp(cstr, "'") == 0)
	{
		putchar('\n');
		RenderString((String *)pParams[0]);
	}
	else if (strcmp(cstr, "\"") == 0)
	{
		putchar('\n');
		RenderString((String *)pParams[2]);
	}
	else if (strcmp(cstr, "TJ") == 0)
	{
		n = ((Array *)pParams[0])->GetSize();
		for (i = 0; i < n; i++)
		{
			pObj = ((Array *)pParams[0])->GetValue(i);
			if (pObj->GetType() == Object::OBJ_STRING)
				RenderString((String *)pObj);
		}
	}
}

void TextRenderer::RenderCharCodes(const uint16_t *codes, int num)
{
	int i;
	wchar_t *wstr;
	char *str;

	wstr = new wchar_t[num + 1];
	for (i = 0; i < num; ++i)
		wstr[i] = m_pFontData->m_pToUnicode? m_pFontData->m_pToUnicode->Get(codes[i]) : codes[i];
	wstr[i] = '\0';

	str = new char[num * 6 + 1];
	wcstombs(str, wstr, num * 6 + 1);
	delete[] wstr;

	printf("%s\n", str);
}
