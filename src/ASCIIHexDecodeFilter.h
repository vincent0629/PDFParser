#ifndef _ASCIIHEXDECODEFILTER_H_
#define _ASCIIHEXDECODEFILTER_H_

#include "CompoundFilter.h"

class CASCIIHexDecodeFilter : public CCompoundFilter
{
public:
	CASCIIHexDecodeFilter(IInputStream *pSource);
	~CASCIIHexDecodeFilter();

private:
	unsigned char *m_pBuffer;
};

#endif
