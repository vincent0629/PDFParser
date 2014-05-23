#ifndef _PDF_H_
#define _PDF_H_

class CDataInputStream;
class CDictionary;
class IInputStream;
class CName;
class CObject;
class CObjReader;
class CStream;
class CXref;

typedef struct _STrailer
{
	CStream *pStream;
	CDictionary *pDict;
	_STrailer *pPrev;
} STrailer;

class CPDF
{
public:
	CPDF(IInputStream *pSource);
	~CPDF();
	const char *GetVersion(void);
	CXref *GetXref(void);
	STrailer *GetTrailer(void);
	CObject *GetObject(int nNum);
	CObject *GetObject(CObject *pObj);
	IInputStream *GetInputStream(CStream *pStream);
	int GetPageNum(void);
	CDictionary *GetPage(int nIndex);

private:
	CDataInputStream *m_pSource;
	char m_pVersion[10];
	CXref *m_pXref;
	CObjReader *m_pReader;
	STrailer *m_pTrailer, *m_pTail;
	int m_nPageNum;
	CDictionary *m_pPages;

	CDictionary *GetPage(CDictionary *pParent, int nIndex);
	IInputStream *CreateFilter(CName *pName, IInputStream *pSource);
};

#endif
