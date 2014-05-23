#ifndef _INPUTSTREAM_H_
#define _INPUTSTREAM_H_

class IInputStream
{
public:
	virtual ~IInputStream() {}
	virtual unsigned int Available(void) = 0;
	virtual unsigned int Position(void) = 0;
	virtual int Seek(int nOffset, int nFrom) = 0;
	virtual int Read(void) = 0;
	virtual int Read(void *pBuffer, int nSize) = 0;
};

#endif
