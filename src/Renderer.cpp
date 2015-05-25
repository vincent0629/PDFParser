#include "Renderer.h"
#include "DataInputStream.h"
#include "FontData.h"
#include "Object.h"
#include "ObjReader.h"
#include "PDF.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

using namespace std;

Renderer::Renderer(PDF *pPDF)
{
	m_pPDF = pPDF;
}

Renderer::~Renderer()
{
}

void Renderer::Render(int nPage)
{
	const Dictionary *pPage;
	const Array *pMediaBox;
	double width, height;
	map<const Object *, FontData *>::iterator it;

	pPage = m_pPDF->GetPage(nPage);
	if (pPage == NULL)
		return;

	m_nPage = nPage;
	m_pResourceRoot = (const Dictionary *)m_pPDF->GetObject(pPage->GetValue("Resources"));
	memset(m_pResources, 0, sizeof(Dictionary *) * RES_NUM);
	m_bStop = false;

	pMediaBox = (const Array *)pPage->GetValue("MediaBox");
	if (!pMediaBox)
	{
		width = 600;
		height = 800;
	}
	else
	{
		width = ((const Numeric *)pMediaBox->GetValue(2))->GetValue();
		height = ((const Numeric *)pMediaBox->GetValue(3))->GetValue();
	}

	RenderPage(pPage, width, height);

	for (it = m_fontDataMap.begin(); it != m_fontDataMap.end(); ++it)
		delete it->second;
	m_fontDataMap.clear();
	delete pPage;
}

void Renderer::Stop(void)
{
	m_bStop = true;
}

const Object *Renderer::GetResource(ResourceType nRes, const char *pName)
{
	const char *pNames[] = {"ColorSpace", "Font", "ExtGState", "XObject"};

	if (m_pResources[nRes] == NULL && m_pResourceRoot != NULL)
		m_pResources[nRes] = (const Dictionary *)m_pPDF->GetObject(m_pResourceRoot->GetValue(pNames[nRes]));
	return m_pResources[nRes] == NULL? NULL : m_pPDF->GetObject(m_pResources[nRes]->GetValue(pName));
}

void Renderer::RenderPage(const Dictionary *pPage, double dWidth, double dHeight)
{
	const Object *pObj;
	int i, n;

	pObj = m_pPDF->GetObject(pPage->GetValue("Contents"));
	if (pObj->GetType() == Object::OBJ_STREAM)
		RenderContents((const Stream *)pObj);
	else if (pObj->GetType() == Object::OBJ_ARRAY)
	{
		n = ((const Array *)pObj)->GetSize();
		for (i = 0; i < n && !m_bStop; i++)
			RenderContents((const Stream *)m_pPDF->GetObject(((const Array *)pObj)->GetValue(i)));
	}
}

void Renderer::RenderContents(const Stream *pContents)
{
	InputStream *pSource;
	DataInputStream *pDIS;
	ObjReader *pReader;
	int nParams;
	const Object *pObj;
	const Object *pParams[10];

	pSource = m_pPDF->CreateInputStream(pContents);
	pDIS = new DataInputStream(pSource);
	pReader = new ObjReader(pDIS, m_pPDF->GetXref());
	nParams = 0;
	while (!m_bStop && pDIS->Available() > 0)
	{
		pObj = pReader->ReadObj();
		if (pObj == NULL)
			break;
		if (pObj->GetType() == Object::OBJ_OPERATOR)
		{
			RenderOperator((const Operator *)pObj, pParams, nParams);
			delete pObj;
			while (nParams > 0)
				delete pParams[--nParams];
		}
		else if (pObj->GetType() != Object::OBJ_INVALID)
			pParams[nParams++] = pObj;
	}
	delete pReader;
	delete pDIS;
	delete pSource;
}

void Renderer::RenderOperator(const Operator *pOp, const Object **pParams, int nParams)
{
}

void Renderer::ChangeFont(const char *pName)
{
	const Dictionary *pFont;
	map<const Object *, FontData *>::iterator it;

	pFont = (const Dictionary *)GetResource(FONT, pName);
	assert(pFont);

	it = m_fontDataMap.find(pFont);
	if (it != m_fontDataMap.end())
		m_pFontData = it->second;
	else
	{
		m_pFontData = new FontData(m_pPDF, pFont);
		m_fontDataMap.insert(make_pair(pFont, m_pFontData));
	}
}

void Renderer::RenderString(const String *pString)
{
}
