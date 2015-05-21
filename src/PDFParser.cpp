#include "FileInputStream.h"
#include "Object.h"
#include "PDF.h"
#include "Renderer.h"
#include "RendererFactory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

static void Run(const char *pFile, RendererFactory::RendererType nType)
{
	FileInputStream *pSource;
	PDF *pPDF;
	const char *pVersion;
	Renderer *pRenderer;
	char str[64], *token[5];
	int n;
	Trailer *pTrailer;
	Object *pObj;
	InputStream *pIS;

	pSource = new FileInputStream(pFile);
	pPDF = new PDF(pSource);
	pVersion = pPDF->GetVersion();
	if (*pVersion == '\0')
		printf("Invalid PDF.\n");
	else
	{
		printf("PDF: %s\n%d pages\n", pVersion, pPDF->GetPageNum());

		pRenderer = RendererFactory::Create(nType, pPDF);

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
					Object::Print(pTrailer->pDict);
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
					Object::Print(pObj);
					printf("\n");
					if (pObj->GetType() == Object::OBJ_STREAM && n >= 3)
					{
						pIS = pPDF->CreateInputStream((Stream *)pObj);
						printf("----- begin %d bytes -----\n", pIS->Available());
						while ((n = pIS->Read(str, sizeof(str) - 1)) > 0)
						{
							str[n] = '\0';
							printf("%s", str);
						}
						printf("----- end -----\n");
						delete pIS;
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
					Object::Print(pObj);
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
					if (pObj->GetType() == Object::OBJ_STREAM)
						printf("Stream #%d size=%d\n", n, ((Stream *)pObj)->GetSize());
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
	RendererFactory::RendererType nType;

	if (argc == 1)
		return 0;

	setlocale(LC_CTYPE, "");
	nType = RendererFactory::TEXT;
	for (i = 1; i < argc; i++)
	{
		if (*argv[i] == '-')
		{
			if (strcmp(argv[i], "-r") == 0)
			{
				++i;
				if (*argv[i] == 'o')
					nType = RendererFactory::OBJECT;
				else if (*argv[i] == 'r')
					nType = RendererFactory::RAW;
				else if (*argv[i] == 'c')
					nType = RendererFactory::CAIRO;
				else
					nType = RendererFactory::TEXT;
			}
		}
		else
			pFile = argv[i];
	}
	Run(pFile, nType);
	return 0;
}
