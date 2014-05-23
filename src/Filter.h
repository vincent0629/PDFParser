#ifndef _FILTER_H_
#define _FILTER_H_

#include "InputStream.h"

class CFilter : public IInputStream
{
public:
	CFilter(IInputStream *pSource);
	virtual ~CFilter();

protected:
	IInputStream *m_pSource;
};

#endif
