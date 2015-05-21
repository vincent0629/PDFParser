#ifndef _FILEINPUTSTREAM_H_
#define _FILEINPUTSTREAM_H_

#include "CompoundInputStream.h"

class FileInputStream : public CompoundInputStream
{
public:
	FileInputStream(const char *pFile);
	~FileInputStream();

private:
	int m_fd;
	unsigned char *m_pData;
	int m_nSize;
};

#endif
