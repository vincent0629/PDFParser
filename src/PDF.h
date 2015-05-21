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
	Stream *pStream;
	Dictionary *pDict;
	_Trailer *pPrev;
} Trailer;

class PDF
{
public:
	PDF(InputStream *pSource);
	~PDF();
	const char *GetVersion(void);
	Xref *GetXref(void);
	Trailer *GetTrailer(void);
	Object *GetObject(int nNum);
	Object *GetObject(Object *pObj);
	InputStream *CreateInputStream(Stream *pStream);
	int GetPageNum(void);
	Dictionary *GetPage(int nIndex);

private:
	DataInputStream *m_pSource;
	char m_pVersion[10];
	Xref *m_pXref;
	ObjReader *m_pReader;
	Trailer *m_pTrailer, *m_pTail;
	int m_nPageNum;
	Dictionary *m_pPages;

	Dictionary *GetPage(Dictionary *pParent, int nIndex);
	InputStream *CreateFilter(Name *pName, InputStream *pSource);
};

#endif
