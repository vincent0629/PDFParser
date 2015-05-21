#ifndef _CMAP_H_
#define _CMAP_H_

class CMap
{
public:
	CMap();
	~CMap();
	void Set(unsigned int nFrom, unsigned int nTo);
	unsigned int Get(unsigned int nCode);
	void Concat(CMap *pCMap);

private:
	unsigned int m_pTable[0x10000];
	CMap *m_pSecond;
};

#endif
