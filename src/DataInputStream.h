#ifndef _DATAINPUTSTREAM_H_
#define _DATAINPUTSTREAM_H_

#include "CompoundInputStream.h"

class CDataInputStream : public CCompoundInputStream
{
public:
	CDataInputStream(IInputStream *pSource);
	int ReadStr(char *pBuffer, int nSize);
	int ReadInt(void);
	void Skip(void);
};

#endif
