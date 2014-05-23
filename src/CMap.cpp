#include "CMap.h"
#include <string.h>

CCMap::CCMap()
{
	memset(m_pTable, 0, sizeof(m_pTable));
	m_pSecond = NULL;
}

CCMap::~CCMap()
{
	delete m_pSecond;
}

void CCMap::Set(unsigned int nFrom, unsigned int nTo)
{
	m_pTable[nFrom] = nTo;
}

unsigned int CCMap::Get(unsigned int nCode)
{
	if (m_pTable[nCode] != 0)
		nCode = m_pTable[nCode];
	if (m_pSecond != NULL)
		nCode = m_pSecond->Get(nCode);
	return nCode;
}

void CCMap::Concat(CCMap *pCMap)
{
	m_pSecond = pCMap;
}
