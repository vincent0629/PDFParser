#include "PDF.h"
#include "ByteArrayInputStream.h"
#include "DataInputStream.h"
#include "FilterFactory.h"
#include "Object.h"
#include "ObjReader.h"
#include "Xref.h"
#include <stdio.h>
#include <string.h>

CPDF::CPDF(IInputStream *pSource)
{
	char str[10];
	int nOffset, c, n;
	CDictionary *pDict;
	STrailer *pTrailer;
	CObject *pObj;

	m_pSource = new CDataInputStream(pSource);
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

	m_pSource->Seek(-30, SEEK_END);
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

	m_pXref = new CXref();
	m_pReader = new CObjReader(m_pSource, m_pXref);
	m_pTrailer = NULL;
	do
	{
		pTrailer = new STrailer;
		m_pSource->Seek(nOffset, SEEK_SET);
		m_pSource->ReadStr(str, sizeof(str));
		if (strcmp(str, "xref") == 0)
		{
			m_pXref->Read(m_pSource);
			m_pSource->ReadStr(str, sizeof(str));  //trailer
			pTrailer->pStream = NULL;
			pTrailer->pDict = (CDictionary *)m_pReader->ReadObj();
		}
		else
		{
			m_pSource->ReadInt();
			m_pSource->ReadStr(str, sizeof(str));  //obj
			pTrailer->pStream = (CStream *)m_pReader->ReadObj();  //stream
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
			nOffset = ((CNumeric *)pObj)->GetValue();
	} while (pObj != NULL);

	m_nPageNum = 0;
	for (pTrailer = m_pTrailer; pTrailer != NULL; pTrailer = pTrailer->pPrev)
	{
		pObj = GetObject(pTrailer->pDict->GetValue("Root"));
		if (pObj != NULL)
		{
			m_pPages = (CDictionary *)GetObject(((CDictionary *)pObj)->GetValue("Pages"));
			m_nPageNum = ((CNumeric *)GetObject(m_pPages->GetValue("Count")))->GetValue();
			break;
		}
	}
}

CPDF::~CPDF()
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

const char *CPDF::GetVersion(void)
{
	return m_pVersion;
}

CXref *CPDF::GetXref(void)
{
	return m_pXref;
}

STrailer *CPDF::GetTrailer(void)
{
	return m_pTrailer;
}

CObject *CPDF::GetObject(int nNum)
{
	return m_pReader->ReadIndirectObj(nNum, 0);
}

CObject *CPDF::GetObject(CObject *pObj)
{
	CObject *pTarget;

	while (pObj != NULL && pObj->GetType() == CObject::OBJ_REFERENCE)
	{
		pTarget = ((CReference *)pObj)->GetObject();
		if (pTarget == NULL)
		{
			pTarget = GetObject(((CReference *)pObj)->GetObjNum());
			((CReference *)pObj)->SetObject(pTarget);
		}
		pObj = pTarget;
	}
	return pObj;
}

IInputStream *CPDF::CreateInputStream(CStream *pStream)
{
	IInputStream *pSource;
	CDictionary *pDict;
	CObject *pFilter, *pParms;
	int i, n;

	pSource = new CByteArrayInputStream(pStream->GetValue(), pStream->GetSize());
	pDict = pStream->GetDictionary();
	pFilter = GetObject(pDict->GetValue("Filter"));
	if (pFilter != NULL)
	{
		pParms = GetObject(pDict->GetValue("DecodeParms"));
		if (pFilter->GetType() == CObject::OBJ_NAME)
			pSource = CFilterFactory::Create(((CName *)pFilter)->GetValue(), (CDictionary *)pParms, pSource);
		else if (pFilter->GetType() == CObject::OBJ_ARRAY)
		{
			n = ((CArray *)pFilter)->GetSize();
			for (i = 0; i < n; i++)
				pSource = CFilterFactory::Create(((CName *)GetObject(((CArray *)pFilter)->GetValue(i)))->GetValue(), pParms == NULL? NULL : (CDictionary *)((CArray *)pParms)->GetValue(i), pSource);
		}
	}
	return pSource;
}

int CPDF::GetPageNum(void)
{
	return m_nPageNum;
}

CDictionary *CPDF::GetPage(int nIndex)
{
	return nIndex > 0? GetPage(m_pPages, nIndex) : NULL;
}

CDictionary *CPDF::GetPage(CDictionary *pParent, int nIndex)
{
	CArray *pKids;
	int i, nSize, nCount;
	CReference *pRef;
	CDictionary *pChild, *pRet;

	pKids = (CArray *)pParent->GetValue("Kids");
	nSize = pKids->GetSize();
	for (i = 0; i < nSize; i++)
	{
		pRef = (CReference *)pKids->GetValue(i);
		pChild = (CDictionary *)GetObject(pRef->GetObjNum());
		if (strcmp(((CName *)pChild->GetValue("Type"))->GetValue(), "Pages") == 0)  //not leaf node
		{
			nCount = ((CNumeric *)pChild->GetValue("Count"))->GetValue();
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
