#ifndef _PREDICTORFILTER_H_
#define _PREDICTORFILTER_H_

#include "Filter.h"

class CPredictorFilter : public CFilter
{
public:
	CPredictorFilter(IInputStream *pSource, int nColors, int nBitsPerComponent, int nColumns);
	~CPredictorFilter();
	unsigned int Available(void);
	unsigned int Position(void);
	int Seek(int nOffset, int nFrom);
	int Read(void);
	int Read(void *pBuffer, int nSize);

private:
	int m_nBpp, m_nRowLength;
};

#endif
