#ifndef _ASCIIHEXDECODEFILTER_H_
#define _ASCIIHEXDECODEFILTER_H_

#include "CompoundFilter.h"

class ASCIIHexDecodeFilter : public CompoundFilter
{
public:
	ASCIIHexDecodeFilter(InputStream *pSource);
	~ASCIIHexDecodeFilter();

private:
	unsigned char *m_pBuffer;
};

#endif
