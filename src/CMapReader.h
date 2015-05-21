#ifndef _CMAPREADER_H_
#define _CMAPREADER_H_

class CMap;
class InputStream;

class CMapReader
{
public:
	CMapReader(CMap *pCMap);
	void Read(InputStream *pSource);

private:
	CMap *m_pCMap;
	int m_nCount;
};

#endif
