#ifndef _COMPOUNDFILTER_H_
#define _COMPOUNDFILTER_H_

#include "Filter.h"

class CCompoundFilter : public CFilter
{
public:
	CCompoundFilter(IInputStream *pSource);
	virtual unsigned int Available(void);
	virtual unsigned int Position(void);
	virtual int Seek(int nOffset, int nFrom);
	virtual int Read(void);
	virtual int Read(void *pBuffer, int nSize);

protected:
	IInputStream *m_pStream;
};

#endif
