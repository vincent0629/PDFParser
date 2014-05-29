#include "FileInputStream.h"
#include "Object.h"
#include "PDF.h"
#include "Renderer.h"
#include "RendererFactory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

static void Run(const char *pFile, CRendererFactory::Renderer_t nType)
{
	CFileInputStream *pSource;
	CPDF *pPDF;
	const char *pVersion;
	CRenderer *pRenderer;
	char str[64], *token[5];
	int n;
	STrailer *pTrailer;
	CObject *pObj;
	IInputStream *pIS;

	pSource = new CFileInputStream(pFile);
	pPDF = new CPDF(pSource);
	pVersion = pPDF->GetVersion();
	if (*pVersion == '\0')
		printf("Invalid PDF.\n");
	else
	{
		printf("PDF: %s\n%d pages\n", pVersion, pPDF->GetPageNum());

		pRenderer = CRendererFactory::Create(nType, pPDF);

		while (true)
		{
			printf("\nPDF> ");
			gets(str);
			token[0] = strtok(str, " ");
			for (n = 1; n < 5; n++)
			{
				token[n] = strtok(NULL, " ");
				if (token[n] == NULL)
					break;
			}

			if (token[0] == NULL)
				continue;
			if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "q") == 0)
				break;
			else if (strcmp(token[0], "trailer") == 0 || strcmp(token[0], "t") == 0)
			{
				pTrailer = pPDF->GetTrailer();
				while (pTrailer != NULL)
				{
					CObject::Print(pTrailer->pDict);
					printf("\n");
					pTrailer = pTrailer->pPrev;
				}
			}
			else if ((strcmp(token[0], "obj") == 0 || strcmp(token[0], "o") == 0) && n >= 2)
			{
				pObj = pPDF->GetObject(atoi(token[1]));
				if (pObj == NULL)
					printf("Object doesn't exist.\n");
				else
				{
					printf("offset = %u\n", pObj->GetOffset());
					CObject::Print(pObj);
					printf("\n");
					if (pObj->GetType() == CObject::OBJ_STREAM && n >= 3)
					{
						printf("----- begin -----\n");
						pIS = pPDF->CreateInputStream((CStream *)pObj);
						while ((n = pIS->Read(str, sizeof(str) - 1)) > 0)
						{
							str[n] = '\0';
							printf("%s", str);
						}
						delete pIS;
						printf("----- end -----\n");
					}
					delete pObj;
				}
			}
			else if ((strcmp(token[0], "page") == 0 || strcmp(token[0], "p") == 0) && n >= 2)
			{
				pObj = pPDF->GetPage(atoi(token[1]));
				if (pObj == NULL)
					printf("Page doesn't exist.\n");
				else
				{
					CObject::Print(pObj);
					delete pObj;

					printf("\n");
					pRenderer->Render(atoi(token[1]));
				}
			}
			else if (strcmp(token[0], "stream") == 0)
			{
				n = 1;
				while (true)
				{
					pObj = pPDF->GetObject(n);
					if (pObj == NULL)
						break;
					if (pObj->GetType() == CObject::OBJ_STREAM)
						printf("Stream #%d size=%d\n", n, ((CStream *)pObj)->GetSize());
					delete pObj;
					++n;
				}
			}
		}

		delete pRenderer;
	}
	delete pPDF;
	delete pSource;
}

int main(int argc, char *argv[])
{
	int i;
	const char *pFile;
	CRendererFactory::Renderer_t nType;

	if (argc == 1)
		return 0;

	setlocale(LC_CTYPE, "");
	nType = CRendererFactory::TEXT;
	for (i = 1; i < argc; i++)
	{
		if (*argv[i] == '-')
		{
			if (strcmp(argv[i], "-r") == 0)
			{
				++i;
				if (*argv[i] == 'o')
					nType = CRendererFactory::OBJECT;
				else if (*argv[i] == 'r')
					nType = CRendererFactory::RAW;
				else if (*argv[i] == 'c')
					nType = CRendererFactory::CAIRO;
				else
					nType = CRendererFactory::TEXT;
			}
		}
		else
			pFile = argv[i];
	}
	Run(pFile, nType);
	return 0;
}
