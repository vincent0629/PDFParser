#ifndef _DATAINPUTSTREAM_H_
#define _DATAINPUTSTREAM_H_

#include "CompoundInputStream.h"

class DataInputStream : public CompoundInputStream
{
public:
	DataInputStream(InputStream *pSource);
	int ReadStr(char *pBuffer, int nSize);
	int ReadInt(void);
	void Skip(void);
};

#endif
