#include "PredictorFilter.h"

CPredictorFilter::CPredictorFilter(IInputStream *pSource, int nColors, int nBitsPerComponent, int nColumns) : CFilter(pSource)
{
	m_nBpp = (nColors * nBitsPerComponent + 7) / 8;
	m_nRowLength = (nColumns * nColors * nBitsPerComponent + 7) / 8 + m_nBpp;
}

CPredictorFilter::~CPredictorFilter()
{
	delete m_pSource;
}

unsigned int CPredictorFilter::Available(void)
{
}

unsigned int CPredictorFilter::Position(void)
{
}

int CPredictorFilter::Seek(int nOffset, int nFrom)
{
}

int CPredictorFilter::Read(void)
{
}

int CPredictorFilter::Read(void *pBuffer, int nSize)
{
}
