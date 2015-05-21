#include "CompoundFilter.h"

CompoundFilter::CompoundFilter(InputStream *pSource) : Filter(pSource)
{
}

unsigned int CompoundFilter::Available(void)
{
	return m_pStream->Available();
}

unsigned int CompoundFilter::Position(void)
{
	return m_pStream->Position();
}

int CompoundFilter::Seek(int nOffset, int nFrom)
{
	return m_pStream->Seek(nOffset, nFrom);
}

int CompoundFilter::Read(void)
{
	return m_pStream->Read();
}

int CompoundFilter::Read(void *pBuffer, int nSize)
{
	return m_pStream->Read(pBuffer, nSize);
}
