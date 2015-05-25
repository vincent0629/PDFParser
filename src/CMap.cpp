#include "CMap.h"
#include <string.h>

CMap::CMap()
{
	memset(m_pTable, 0, sizeof(m_pTable));
	m_pSecond = NULL;
}

CMap::~CMap()
{
	delete m_pSecond;
}

void CMap::Set(unsigned int nFrom, unsigned int nTo)
{
	m_pTable[nFrom] = nTo;
}

unsigned int CMap::Get(unsigned int nCode) const
{
	if (m_pTable[nCode] != 0)
		nCode = m_pTable[nCode];
	if (m_pSecond != NULL)
		nCode = m_pSecond->Get(nCode);
	return nCode;
}

void CMap::Concat(const CMap *pCMap)
{
	m_pSecond = pCMap;
}
