#ifndef _ASCII85DECODEFILTER_H_
#define _ASCII85DECODEFILTER_H_

#include "CompoundFilter.h"

class ASCII85DecodeFilter : public CompoundFilter
{
public:
	ASCII85DecodeFilter(InputStream *pSource);
	~ASCII85DecodeFilter();

private:
	unsigned char *m_pBuffer;
};

#endif
