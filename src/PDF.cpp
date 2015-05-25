#include "PDF.h"
#include "ByteArrayInputStream.h"
#include "DataInputStream.h"
#include "FilterFactory.h"
#include "Object.h"
#include "ObjReader.h"
#include "Xref.h"
#include <stdio.h>
#include <string.h>

PDF::PDF(InputStream *pSource)
{
	char str[10];
	int nOffset, c, n;
	Dictionary *pDict;
	Trailer *pTrailer;
	Object *pObj;

	m_pSource = new DataInputStream(pSource);
	*str = m_pSource->Read();
	m_pSource->ReadStr(str + 1, sizeof(str) - 1);
	if (strncmp(str, "%PDF-", 5) != 0)
	{
		m_pVersion[0] = '\0';
		delete m_pSource;
		m_pSource = NULL;
		return;
	}

	strcpy(m_pVersion, (const char *)str + 5);

	m_pSource->Seek(-1024, SEEK_END);
	while (true)
	{
		c = m_pSource->Read();
		if (c == EOF)
		{
			m_pVersion[0] = '\0';
			delete m_pSource;
			m_pSource = NULL;
			return;
		}
		if (c == 's')
		{
			*str = 's';
			m_pSource->ReadStr(str + 1, sizeof(str) - 1);
			if (strcmp(str, "startxref") == 0)
				break;
		}
	}
	nOffset = m_pSource->ReadInt();

	m_pXref = new Xref();
	m_pReader = new ObjReader(m_pSource, m_pXref);
	m_pTrailer = NULL;
	do
	{
		pTrailer = new Trailer;
		m_pSource->Seek(nOffset, SEEK_SET);
		m_pSource->ReadStr(str, sizeof(str));
		if (strcmp(str, "xref") == 0)
		{
			m_pXref->Read(m_pSource);
			m_pSource->ReadStr(str, sizeof(str));  //trailer
			pTrailer->pStream = NULL;
			pTrailer->pDict = (Dictionary *)m_pReader->ReadObj();
		}
		else
		{
			m_pSource->ReadInt();
			m_pSource->ReadStr(str, sizeof(str));  //obj
			pTrailer->pStream = (Stream *)m_pReader->ReadObj();  //stream
			pTrailer->pDict = pTrailer->pStream->GetDictionary();
			pSource = CreateInputStream(pTrailer->pStream);
			m_pXref->Read(pSource);
			delete pSource;
		}

		pTrailer->pPrev = NULL;
		if (m_pTrailer == NULL)
			m_pTrailer = pTrailer;
		else
			m_pTail->pPrev = pTrailer;
		m_pTail = pTrailer;

		pObj = pTrailer->pDict->GetValue("Prev");
		if (pObj != NULL)
			nOffset = ((Numeric *)pObj)->GetValue();
	} while (pObj != NULL);

	m_nPageNum = 0;
	for (pTrailer = m_pTrailer; pTrailer != NULL; pTrailer = pTrailer->pPrev)
	{
		pObj = GetObject(pTrailer->pDict->GetValue("Root"));
		if (pObj != NULL)
		{
			m_pPages = (Dictionary *)GetObject(((Dictionary *)pObj)->GetValue("Pages"));
			m_nPageNum = ((Numeric *)GetObject(m_pPages->GetValue("Count")))->GetValue();
			break;
		}
	}
}

PDF::~PDF()
{
	if (m_pSource != NULL)
	{
		while (m_pTrailer != NULL)
		{
			m_pTail = m_pTrailer->pPrev;
			if (m_pTrailer->pStream != NULL)
				delete m_pTrailer->pStream;
			else
				delete m_pTrailer->pDict;
			delete m_pTrailer;
			m_pTrailer = m_pTail;
		}
		delete m_pReader;
		delete m_pXref;
		delete m_pSource;
	}
}

const char *PDF::GetVersion(void)
{
	return m_pVersion;
}

Xref *PDF::GetXref(void)
{
	return m_pXref;
}

Trailer *PDF::GetTrailer(void)
{
	return m_pTrailer;
}

Object *PDF::GetObject(int nNum)
{
	return m_pReader->ReadIndirectObj(nNum, 0);
}

Object *PDF::GetObject(Object *pObj)
{
	Object *pTarget;

	while (pObj != NULL && pObj->GetType() == Object::OBJ_REFERENCE)
	{
		pTarget = ((Reference *)pObj)->GetObject();
		if (pTarget == NULL)
		{
			pTarget = GetObject(((Reference *)pObj)->GetObjNum());
			((Reference *)pObj)->SetObject(pTarget);
		}
		pObj = pTarget;
	}
	return pObj;
}

InputStream *PDF::CreateInputStream(Stream *pStream)
{
	InputStream *pSource;
	Dictionary *pDict;
	Object *pFilter, *pParms;
	int i, n;

	pSource = new ByteArrayInputStream(pStream->GetValue(), pStream->GetSize());
	pDict = pStream->GetDictionary();
	pFilter = GetObject(pDict->GetValue("Filter"));
	if (pFilter != NULL)
	{
		pParms = GetObject(pDict->GetValue("DecodeParms"));
		if (pFilter->GetType() == Object::OBJ_NAME)
			pSource = FilterFactory::Create(((Name *)pFilter)->GetValue(), (Dictionary *)pParms, pSource);
		else if (pFilter->GetType() == Object::OBJ_ARRAY)
		{
			n = ((Array *)pFilter)->GetSize();
			for (i = 0; i < n; i++)
				pSource = FilterFactory::Create(((Name *)GetObject(((Array *)pFilter)->GetValue(i)))->GetValue(), pParms == NULL? NULL : (Dictionary *)((Array *)pParms)->GetValue(i), pSource);
		}
	}
	return pSource;
}

int PDF::GetPageNum(void)
{
	return m_nPageNum;
}

Dictionary *PDF::GetPage(int nIndex)
{
	return nIndex > 0? GetPage(m_pPages, nIndex) : NULL;
}

Dictionary *PDF::GetPage(Dictionary *pParent, int nIndex)
{
	Array *pKids;
	int i, nSize, nCount;
	Reference *pRef;
	Dictionary *pChild, *pRet;

	pKids = (Array *)pParent->GetValue("Kids");
	nSize = pKids->GetSize();
	for (i = 0; i < nSize; i++)
	{
		pRef = (Reference *)pKids->GetValue(i);
		pChild = (Dictionary *)GetObject(pRef->GetObjNum());
		if (strcmp(((Name *)pChild->GetValue("Type"))->GetValue(), "Pages") == 0)  //not leaf node
		{
			nCount = ((Numeric *)pChild->GetValue("Count"))->GetValue();
			if (nIndex <= nCount)
			{
				pRet = GetPage(pChild, nIndex);
				delete pChild;
				return pRet;
			}
			else
				nIndex -= nCount;
		}
		else if (--nIndex == 0)
			return pChild;
		delete pChild;
	}
	return NULL;
}
