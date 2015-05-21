#include "CompoundInputStream.h"

CompoundInputStream::~CompoundInputStream()
{
}

unsigned int CompoundInputStream::Available(void)
{
	return m_pSource->Available();
}

unsigned int CompoundInputStream::Position(void)
{
	return m_pSource->Position();
}

int CompoundInputStream::Seek(int nOffset, int nFrom)
{
	return m_pSource->Seek(nOffset, nFrom);
}

int CompoundInputStream::Read(void)
{
	return m_pSource->Read();
}

int CompoundInputStream::Read(void *pBuffer, int nSize)
{
	return m_pSource->Read(pBuffer, nSize);
}
