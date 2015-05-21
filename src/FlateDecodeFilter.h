#ifndef _FLATEDECODEFILTER_H_
#define _FLATEDECODEFILTER_H_

#include "CompoundFilter.h"

class FlateDecodeFilter : public CompoundFilter
{
public:
	FlateDecodeFilter(InputStream *pSource);
	~FlateDecodeFilter();

private:
	unsigned char *m_pBuffer;
};

#endif
