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
		OBJ_INVALID,
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

	Object();
	Object(ObjType nType);
	virtual ~Object();
	ObjType GetType(void) const;
	void SetOffset(unsigned int nOffset);
	unsigned int GetOffset(void) const;

public:
	static void Print(const Object *pObj);

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
	bool GetValue(void) const;

private:
	bool m_bValue;
};

class Numeric : public Object
{
public:
	Numeric();
	void SetValue(double dValue);
	double GetValue(void) const;

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
	const char *GetValue(void) const;
	int GetLength(void) const;

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
	const char *GetValue(void) const;

private:
	char *m_pValue;
};

class Array : public Object
{
public:
	Array();
	~Array();
	void Add(const Object *pValue);
	int GetSize(void) const;
	const Object *GetValue(int nIndex) const;

private:
	std::vector<const Object *> m_pValue;
};

class Dictionary : public Object
{
public:
	Dictionary();
	~Dictionary();
	void Add(const Object *pName, const Object *pValue);
	int GetSize(void) const;
	const char *GetName(int nIndex) const;
	const Object *GetValue(const char *pKey) const;

private:
	std::vector<const Object *> m_pName, m_pValue;
};

class Stream : public Object
{
public:
	Stream(const Dictionary *pDict);
	~Stream();
	const Dictionary *GetDictionary() const;
	void SetValue(const unsigned char *pValue, int nSize);
	const unsigned char *GetValue(void) const;
	int GetSize(void) const;

private:
	const Dictionary *m_pDict;
	unsigned char *m_pValue;
	int m_nSize;
};

class Reference : public Object
{
public:
	Reference();
	~Reference();
	void SetValue(int nObjNum, int nGeneration);
	int GetObjNum(void) const;
	int GetGeneration(void) const;
	void SetObject(const Object *pObj);
	const Object *GetObject(void) const;

private:
	int m_nObjNum, m_nGeneration;
	const Object *m_pObj;
};

class Operator : public Object
{
public:
	Operator();
	~Operator();
	void SetValue(const char *pValue);
	const char *GetValue(void) const;

private:
	char *m_pValue;
};

#endif
