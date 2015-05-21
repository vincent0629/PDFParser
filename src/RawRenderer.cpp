#include "RawRenderer.h"
#include "InputStream.h"
#include "PDF.h"
#include <stdio.h>

RawRenderer::RawRenderer(PDF *pPDF) : Renderer(pPDF)
{
}

void RawRenderer::RenderContents(Stream *pContents)
{
	InputStream *pSource;
	char str[256];
	int n;

	pSource = m_pPDF->CreateInputStream(pContents);
	while ((n = pSource->Read(str, sizeof(str))) > 0)
		fwrite(str, 1, n, stdout);
	delete pSource;
}
