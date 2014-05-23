#ifndef _CMAPREADER_H_
#define _CMAPREADER_H_

class CCMap;
class IInputStream;

class CCMapReader
{
public:
	CCMapReader(CCMap *pCMap);
	void Read(IInputStream *pSource);

private:
	CCMap *m_pCMap;
	int m_nCount;
};

#endif
