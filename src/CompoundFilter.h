#ifndef _COMPOUNDFILTER_H_
#define _COMPOUNDFILTER_H_

#include "Filter.h"

class CompoundFilter : public Filter
{
public:
	CompoundFilter(InputStream *pSource);
	virtual unsigned int Available(void);
	virtual unsigned int Position(void);
	virtual int Seek(int nOffset, int nFrom);
	virtual int Read(void);
	virtual int Read(void *pBuffer, int nSize);

protected:
	InputStream *m_pStream;
};

#endif
