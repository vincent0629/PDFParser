#ifndef _FILTER_H_
#define _FILTER_H_

#include "InputStream.h"

class Filter : public InputStream
{
public:
	Filter(InputStream *pSource);
	virtual ~Filter();

protected:
	InputStream *m_pSource;
};

#endif
