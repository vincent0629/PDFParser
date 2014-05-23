#include "CompoundInputStream.h"

CCompoundInputStream::~CCompoundInputStream()
{
}

unsigned int CCompoundInputStream::Available(void)
{
	return m_pSource->Available();
}

unsigned int CCompoundInputStream::Position(void)
{
	return m_pSource->Position();
}

int CCompoundInputStream::Seek(int nOffset, int nFrom)
{
	return m_pSource->Seek(nOffset, nFrom);
}

int CCompoundInputStream::Read(void)
{
	return m_pSource->Read();
}

int CCompoundInputStream::Read(void *pBuffer, int nSize)
{
	return m_pSource->Read(pBuffer, nSize);
}
