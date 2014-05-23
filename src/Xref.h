#ifndef _XREF_H_
#define _XREF_H_

class IInputStream;
struct _SSubSection;

class CXref
{
public:
	CXref();
	~CXref();
	void Read(IInputStream *pSource);
	unsigned int GetOffset(int nObjNum, int nGeneration);

private:
	_SSubSection *m_pHead;
};

#endif
