#ifndef _COMPOUNDINPUTSTREAM_H_
#define _COMPOUNDINPUTSTREAM_H_

#include "InputStream.h"

class CCompoundInputStream : public IInputStream
{
public:
	virtual ~CCompoundInputStream();
	virtual unsigned int Available(void);
	virtual unsigned int Position(void);
	virtual int Seek(int nOffset, int nFrom);
	virtual int Read(void);
	virtual int Read(void *pBuffer, int nSize);

protected:
	IInputStream *m_pSource;
};

#endif
