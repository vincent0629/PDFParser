#include "PredictorFilter.h"

PredictorFilter::PredictorFilter(InputStream *pSource, int nColors, int nBitsPerComponent, int nColumns) : Filter(pSource)
{
	m_nBpp = (nColors * nBitsPerComponent + 7) / 8;
	m_nRowLength = (nColumns * nColors * nBitsPerComponent + 7) / 8 + m_nBpp;
}

unsigned int PredictorFilter::Available(void)
{
	//not implemented
	return 0;
}

unsigned int PredictorFilter::Position(void)
{
	//not implemented
	return 0;
}

int PredictorFilter::Seek(int nOffset, int nFrom)
{
	//not implemented
	return 0;
}

int PredictorFilter::Read(void)
{
	//not implemented
	return 0;
}

int PredictorFilter::Read(void *pBuffer, int nSize)
{
	//not implemented
	return 0;
}
