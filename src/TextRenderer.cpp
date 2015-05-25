#include "TextRenderer.h"
#include "CMap.h"
#include "FontData.h"
#include "Object.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

TextRenderer::TextRenderer(PDF *pPDF) : Renderer(pPDF)
{
}

void TextRenderer::RenderOperator(const Operator *pOp, const Object **pParams, int nParams)
{
	const char *cstr;
	int i, n, index;
	const Object *pObj;

	cstr = ((const Operator *)pOp)->GetValue();
	if (strcmp(cstr, "Tf") == 0)
		ChangeFont(((const Name *)pParams[0])->GetValue());
	else if (strcmp(cstr, "TD") == 0 || strcmp(cstr, "T*") == 0)
		putchar('\n');
	else if (strcmp(cstr, "Tj") == 0)
		RenderString((const String *)pParams[0]);
	else if (strcmp(cstr, "'") == 0)
	{
		putchar('\n');
		RenderString((const String *)pParams[0]);
	}
	else if (strcmp(cstr, "\"") == 0)
	{
		putchar('\n');
		RenderString((const String *)pParams[2]);
	}
	else if (strcmp(cstr, "TJ") == 0)
	{
		n = ((const Array *)pParams[0])->GetSize();
		for (i = 0; i < n; i++)
		{
			pObj = ((const Array *)pParams[0])->GetValue(i);
			if (pObj->GetType() == Object::OBJ_STRING)
				RenderString((const String *)pObj);
		}
	}
}

void TextRenderer::RenderString(const String *pString)
{
	int n;
	wchar_t *wstr;
	char *str;

	n = pString->GetLength();
	wstr = new wchar_t[n + 1];
	n = m_pFontData->CharCodesToUnicodes(pString->GetValue(), n, wstr);
	wstr[n] = L'\0';

	str = new char[n * 6 + 1];
	wcstombs(str, wstr, n * 6 + 1);
	printf("%s\n", str);
	delete[] str;

	delete[] wstr;
}
