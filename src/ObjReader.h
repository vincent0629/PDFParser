#ifndef _OBJREADER_H_
#define _OBJREADER_H_

class CDataInputStream;
class CObject;
class CXref;

class CObjReader
{
public:
	CObjReader(CDataInputStream *pSource, CXref *pXref);
	CObject *ReadObj(void);
	CObject *ReadIndirectObj(int nObjNum, int nGeneration);

private:
	CDataInputStream *m_pSource;
	CXref *m_pXref;
};

#endif
