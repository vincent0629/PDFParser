#include "RawRenderer.h"
#include "InputStream.h"
#include "PDF.h"
#include <stdio.h>

CRawRenderer::CRawRenderer(CPDF *pPDF) : CRenderer(pPDF)
{
}

void CRawRenderer::RenderContents(CStream *pContents)
{
	IInputStream *pSource;
	char str[256];
	int n;

	pSource = m_pPDF->CreateInputStream(pContents);
	while ((n = pSource->Read(str, sizeof(str))) > 0)
		fwrite(str, 1, n, stdout);
	delete pSource;
}
