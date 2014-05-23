#ifndef _CMAP_H_
#define _CMAP_H_

class CCMap
{
public:
	CCMap();
	~CCMap();
	void Set(unsigned int nFrom, unsigned int nTo);
	unsigned int Get(unsigned int nCode);
	void Concat(CCMap *pCMap);

private:
	unsigned int m_pTable[0x10000];
	CCMap *m_pSecond;
};

#endif
