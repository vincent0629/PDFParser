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
		PrintObj(pParams[i]);
		putchar(' ');
	}
	PrintObj(pOp);
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
	printf("%s", str);
}

void CObjRenderer::PrintObj(CObject *pObj)
{
	double d;
	int i, n;
	const char *pName;

	switch (pObj->GetType())
	{
		case CObject::OBJ_NULL:
			printf("null");
			break;
		case CObject::OBJ_BOOLEAN:
			printf("%s", ((CBoolean *)pObj)->GetValue()? "true" : "false");
			break;
		case CObject::OBJ_NUMERIC:
			d = ((CNumeric *)pObj)->GetValue();
			if (d == (int)d)
				printf("%d", (int)d);
			else
				printf("%lf", d);
			break;
		case CObject::OBJ_STRING:
			printf("\"");
			if (m_beginText)
				RenderText((CString *)pObj);
			else
				printf("%s", ((CString *)pObj)->GetValue());  // when will this happen?
			printf("\"");
			break;
		case CObject::OBJ_NAME:
			printf("/%s", ((CName *)pObj)->GetValue());
			break;
		case CObject::OBJ_ARRAY:
			printf("[");
			n = ((CArray *)pObj)->GetSize();
			for (i = 0; i < n; i++)
			{
				if (i > 0)
					printf(" ");
				PrintObj(((CArray *)pObj)->GetValue(i));
			}
			printf("]");
			break;
		case CObject::OBJ_DICTIONARY:
			printf("<\n");
			n = ((CDictionary *)pObj)->GetSize();
			for (i = 0; i < n; i++)
			{
				pName = ((CDictionary *)pObj)->GetName(i);
				printf(" %s: ", pName);
				PrintObj(((CDictionary *)pObj)->GetValue(pName));
				printf("\n");
			}
			printf(">");
			break;
		case CObject::OBJ_STREAM:
			PrintObj(((CStream *)pObj)->GetDictionary());
			printf("\nstream %d", ((CStream *)pObj)->GetSize());
			break;
		case CObject::OBJ_REFERENCE:
			printf("%d %d R", ((CReference *)pObj)->GetObjNum(), ((CReference *)pObj)->GetGeneration());
			break;
		case CObject::OBJ_OPERATOR:
			printf("%s", ((COperator *)pObj)->GetValue());
			break;
	}
}
