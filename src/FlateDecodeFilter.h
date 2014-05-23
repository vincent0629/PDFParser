#ifndef _FLATEDECODEFILTER_H_
#define _FLATEDECODEFILTER_H_

#include "CompoundFilter.h"

class CFlateDecodeFilter : public CCompoundFilter
{
public:
	CFlateDecodeFilter(IInputStream *pSource);
	~CFlateDecodeFilter();

private:
	unsigned char *m_pBuffer;
};

#endif
