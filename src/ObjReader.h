#ifndef _OBJREADER_H_
#define _OBJREADER_H_

class DataInputStream;
class Object;
class Xref;

class ObjReader
{
public:
	ObjReader(DataInputStream *pSource, Xref *pXref);
	Object *ReadObj(void);
	Object *ReadIndirectObj(int nObjNum, int nGeneration);

private:
	DataInputStream *m_pSource;
	Xref *m_pXref;
};

#endif
