#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <string>
#include <vector>

class DataInputStream;

class Object
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
	} ObjType;

	Object(ObjType nType);
	virtual ~Object();
	ObjType GetType(void);
	void SetOffset(unsigned int nOffset);
	unsigned int GetOffset(void);

public:
	static void Print(Object *pObj);

private:
	ObjType m_nType;
	unsigned int m_nOffset;
};

class Null : public Object
{
public:
	Null();
};

class Boolean : public Object
{
public:
	Boolean();
	void SetValue(bool bValue);
	bool GetValue(void);

private:
	bool m_bValue;
};

class Numeric : public Object
{
public:
	Numeric();
	void SetValue(double dValue);
	double GetValue(void);

private:
	double m_dValue;
};

class String : public Object
{
public:
	typedef enum
	{
		LITERAL,
		HEXADECIMAL
	} StringFormatType;
	String();
	~String();
	void SetValue(const char *pValue, int nLength, StringFormatType nFormat);
	const char *GetValue(void);
	int GetLength(void);

private:
	char *m_pValue;
	int m_nLength;
};

class Name : public Object
{
public:
	Name();
	~Name();
	void SetValue(const char *pValue);
	const char *GetValue(void);

private:
	char *m_pValue;
};

class Array : public Object
{
public:
	Array();
	~Array();
	void Add(Object *pValue);
	int GetSize(void);
	Object *GetValue(int nIndex);

private:
	std::vector<Object *> m_pValue;
};

class Dictionary : public Object
{
public:
	Dictionary();
	~Dictionary();
	void Add(Object *pName, Object *pValue);
	int GetSize(void);
	const char *GetName(int nIndex);
	Object *GetValue(const char *pKey);

private:
	std::vector<Object *> m_pName, m_pValue;
};

class Stream : public Object
{
public:
	Stream(Dictionary *pDict);
	~Stream();
	Dictionary *GetDictionary();
	void SetValue(const unsigned char *pValue, int nSize);
	const unsigned char *GetValue(void);
	int GetSize(void);

private:
	Dictionary *m_pDict;
	unsigned char *m_pValue;
	int m_nSize;
};

class Reference : public Object
{
public:
	Reference();
	~Reference();
	void SetValue(int nObjNum, int nGeneration);
	int GetObjNum(void);
	int GetGeneration(void);
	void SetObject(Object *pObj);
	Object *GetObject(void);

private:
	int m_nObjNum, m_nGeneration;
	Object *m_pObj;
};

class Operator : public Object
{
public:
	Operator();
	~Operator();
	void SetValue(const char *pValue);
	const char *GetValue(void);

private:
	char *m_pValue;
};

#endif
