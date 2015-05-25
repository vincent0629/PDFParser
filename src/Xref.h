#ifndef _XREF_H_
#define _XREF_H_

class InputStream;
struct _SubSection;

class Xref
{
public:
	Xref();
	~Xref();
	void Read(InputStream *pSource);
	unsigned int GetOffset(int nObjNum, int nGeneration) const;

private:
	_SubSection *m_pHead;
};

#endif
