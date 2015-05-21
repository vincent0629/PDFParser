#ifndef _BYTEARRAYINPUTSTREAM_H_
#define _BYTEARRAYINPUTSTREAM_H_

#include "InputStream.h"

class ByteArrayInputStream : public InputStream
{
public:
	ByteArrayInputStream(const unsigned char *pData, int nSize);
	unsigned int Available(void);
	unsigned int Position(void);
	int Seek(int nOffset, int nFrom);
	int Read(void);
	int Read(void *pBuffer, int nSize);

private:
	const unsigned char *m_pData, *m_ptr;
	int m_nSize;
};

#endif
