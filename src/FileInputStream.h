#ifndef _FILEINPUTSTREAM_H_
#define _FILEINPUTSTREAM_H_

#include "CompoundInputStream.h"

class CFileInputStream : public CCompoundInputStream
{
public:
	CFileInputStream(const char *pFile);
	~CFileInputStream();

private:
	int m_fd;
	unsigned char *m_pData;
	int m_nSize;
};

#endif
