#ifndef _PDF_H_
#define _PDF_H_

class DataInputStream;
class Dictionary;
class InputStream;
class Name;
class Object;
class ObjReader;
class Stream;
class Xref;

typedef struct _Trailer
{
	const Stream *pStream;
	const Dictionary *pDict;
	_Trailer *pPrev;
} Trailer;

class PDF
{
public:
	PDF(InputStream *pSource);
	~PDF();
	const char *GetVersion(void);
	const Xref *GetXref(void);
	const Trailer *GetTrailer(void);
	const Object *GetObject(int nNum);
	const Object *GetObject(const Object *pObj);
	InputStream *CreateInputStream(const Stream *pStream);
	int GetPageNum(void);
	const Dictionary *GetPage(int nIndex);

private:
	DataInputStream *m_pSource;
	char m_pVersion[10];
	Xref *m_pXref;
	ObjReader *m_pReader;
	Trailer *m_pTrailer, *m_pTail;
	int m_nPageNum;
	const Dictionary *m_pPages;

	const Dictionary *GetPage(const Dictionary *pParent, int nIndex);
	InputStream *CreateFilter(const Name *pName, InputStream *pSource);
};

#endif
