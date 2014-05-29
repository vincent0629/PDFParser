#include "PredictorFilter.h"

CPredictorFilter::CPredictorFilter(IInputStream *pSource, int nColors, int nBitsPerComponent, int nColumns) : CFilter(pSource)
{
	m_nBpp = (nColors * nBitsPerComponent + 7) / 8;
	m_nRowLength = (nColumns * nColors * nBitsPerComponent + 7) / 8 + m_nBpp;
}

unsigned int CPredictorFilter::Available(void)
{
	//not implemented
	return 0;
}

unsigned int CPredictorFilter::Position(void)
{
	//not implemented
	return 0;
}

int CPredictorFilter::Seek(int nOffset, int nFrom)
{
	//not implemented
	return 0;
}

int CPredictorFilter::Read(void)
{
	//not implemented
	return 0;
}

int CPredictorFilter::Read(void *pBuffer, int nSize)
{
	//not implemented
	return 0;
}
