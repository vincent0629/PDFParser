#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <string>
#include <vector>

class CDataInputStream;

class CObject
{
public:
	typedef enum
	{
		OBJ_NULL,
		OBJ_BOOLEAN,
		OBJ_NUMERIC,
		OBJ_STRING,
		OBJ_NAME,
		OBJ_ARRAY,
		OBJ_DICTIONARY,
		OBJ_STREAM,
		OBJ_REFERENCE,
		OBJ_OPERATOR
	} ObjType_t;

	CObject(ObjType_t nType);
	virtual ~CObject();
	ObjType_t GetType(void);
	void SetOffset(unsigned int nOffset);
	unsigned int GetOffset(void);

private:
	ObjType_t m_nType;
	unsigned int m_nOffset;
};

class CNull : public CObject
{
public:
	CNull();
};

class CBoolean : public CObject
{
public:
	CBoolean();
	void SetValue(bool bValue);
	bool GetValue(void);

private:
	bool m_bValue;
};

class CNumeric : public CObject
{
public:
	CNumeric();
	void SetValue(double dValue);
	double GetValue(void);

private:
	double m_dValue;
};

class CString : public CObject
{
public:
	typedef enum
	{
		LITERAL,
		HEXADECIMAL
	} StringFormat_t;
	CString();
	~CString();
	void SetValue(const char *pValue, int nLength, StringFormat_t nFormat);
	const char *GetValue(void);
	int GetLength(void);

private:
	char *m_pValue;
	int m_nLength;
};

class CName : public CObject
{
public:
	CName();
	~CName();
	void SetValue(const char *pValue);
	const char *GetValue(void);

private:
	char *m_pValue;
};

class CArray : public CObject
{
public:
	CArray();
	~CArray();
	void Add(CObject *pValue);
	int GetSize(void);
	CObject *GetValue(int nIndex);

private:
	std::vector<CObject *> m_pValue;
};

class CDictionary : public CObject
{
public:
	CDictionary();
	~CDictionary();
	void Add(CObject *pName, CObject *pValue);
	int GetSize(void);
	const char *GetName(int nIndex);
	CObject *GetValue(const char *pKey);

private:
	std::vector<CObject *> m_pName, m_pValue;
};

class CStream : public CObject
{
public:
	CStream(CDictionary *pDict);
	~CStream();
	CDictionary *GetDictionary();
	void SetValue(const unsigned char *pValue, int nSize);
	const unsigned char *GetValue(void);
	int GetSize(void);

private:
	CDictionary *m_pDict;
	unsigned char *m_pValue;
	int m_nSize;
};

class CReference : public CObject
{
public:
	CReference();
	~CReference();
	void SetValue(int nObjNum, int nGeneration);
	int GetObjNum(void);
	int GetGeneration(void);
	void SetObject(CObject *pObj);
	CObject *GetObject(void);

private:
	int m_nObjNum, m_nGeneration;
	CObject *m_pObj;
};

class COperator : public CObject
{
public:
	COperator();
	~COperator();
	void SetValue(const char *pValue);
	const char *GetValue(void);

private:
	char *m_pValue;
};

#endif
