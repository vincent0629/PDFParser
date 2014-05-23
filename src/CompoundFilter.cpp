#include "CompoundFilter.h"

CCompoundFilter::CCompoundFilter(IInputStream *pSource) : CFilter(pSource)
{
}

unsigned int CCompoundFilter::Available(void)
{
	return m_pStream->Available();
}

unsigned int CCompoundFilter::Position(void)
{
	return m_pStream->Position();
}

int CCompoundFilter::Seek(int nOffset, int nFrom)
{
	return m_pStream->Seek(nOffset, nFrom);
}

int CCompoundFilter::Read(void)
{
	return m_pStream->Read();
}

int CCompoundFilter::Read(void *pBuffer, int nSize)
{
	return m_pStream->Read(pBuffer, nSize);
}
