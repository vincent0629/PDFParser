#include "Xref.h"
#include "InputStream.h"
#include <stdio.h>

typedef struct _SubSection
{
	int nBegin, nNum;
	unsigned int *pOffset;
	_SubSection *pNext;
} SubSection;

int ReadInt(InputStream *pSource)
{
	int c, n;

	n = 0;
	while (true)
	{
		c = pSource->Read();
		if (c < '0' || c > '9')
			break;
		n = n * 10 + (c - '0');
	}
	do
	{
		c = pSource->Read();
	} while (c == ' ' || c == '\n' || c == '\r');
	pSource->Seek(-1, SEEK_CUR);
	return n;
}

Xref::Xref()
{
	m_pHead = NULL;
}

Xref::~Xref()
{
	SubSection *pSec;

	while (m_pHead != NULL)
	{
		pSec = m_pHead->pNext;
		delete[] m_pHead->pOffset;
		delete m_pHead;
		m_pHead = pSec;
	}
}

void Xref::Read(InputStream *pSource)
{
	SubSection *pSec;
	int i, c;

	do
	{
		pSec = new SubSection;
		pSec->nBegin = ReadInt(pSource);
		pSec->nNum = ReadInt(pSource);
		pSec->pOffset = new unsigned int[pSec->nNum];
		for (i = 0; i < pSec->nNum; i++)
		{
			pSec->pOffset[i] = ReadInt(pSource);
			pSource->Seek(20 - 10 - 1, SEEK_CUR);
		}
		pSec->pNext = m_pHead;
		m_pHead = pSec;

		do
		{
			c = pSource->Read();
		} while (c == ' ' || c == '\n' || c == '\r');
		pSource->Seek(-1, SEEK_CUR);
	} while (c != 't' && c != EOF);  //trailer
}

unsigned int Xref::GetOffset(int nObjNum, int nGeneration)
{
	SubSection *pSec;

	pSec = m_pHead;
	while (pSec != NULL)
	{
		if (nObjNum >= pSec->nBegin && nObjNum < pSec->nBegin + pSec->nNum)
			return pSec->pOffset[nObjNum - pSec->nBegin];
		pSec = pSec->pNext;
	}
	return 0;
}
