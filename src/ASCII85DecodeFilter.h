#ifndef _ASCII85DECODEFILTER_H_
#define _ASCII85DECODEFILTER_H_

#include "CompoundFilter.h"

class CASCII85DecodeFilter : public CCompoundFilter
{
public:
	CASCII85DecodeFilter(IInputStream *pSource);
	~CASCII85DecodeFilter();

private:
	unsigned char *m_pBuffer;
};

#endif
