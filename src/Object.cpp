#include "Object.h"
#include "DataInputStream.h"
#include <stdio.h>
#include <string.h>
#include <iconv.h>
#include <assert.h>

using namespace std;

static int HtoD(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	assert(false);
	return 0;
}

Object::Object(ObjType nType)
{
	m_nType = nType;
	m_nOffset = -1;
}

Object::~Object()
{
}

Object::ObjType Object::GetType(void)
{
	return m_nType;
}

void Object::SetOffset(unsigned int nOffset)
{
	m_nOffset = nOffset;
}

unsigned int Object::GetOffset(void)
{
	return m_nOffset;
}

void Object::Print(Object *pObj)
{
	double d;
	int i, n;
	const char *pName;

	switch (pObj->GetType())
	{
		case Object::OBJ_NULL:
			printf("null");
			break;
		case Object::OBJ_BOOLEAN:
			printf("%s", ((Boolean *)pObj)->GetValue()? "true" : "false");
			break;
		case Object::OBJ_NUMERIC:
			d = ((Numeric *)pObj)->GetValue();
			if (d == (int)d)
				printf("%d", (int)d);
			else
				printf("%lf", d);
			break;
		case Object::OBJ_STRING:
			printf("\"%s\"", ((String *)pObj)->GetValue());
			break;
		case Object::OBJ_NAME:
			printf("/%s", ((Name *)pObj)->GetValue());
			break;
		case Object::OBJ_ARRAY:
			printf("[");
			n = ((Array *)pObj)->GetSize();
			for (i = 0; i < n; i++)
			{
				if (i > 0)
					printf(" ");
				Print(((Array *)pObj)->GetValue(i));
			}
			printf("]");
			break;
		case Object::OBJ_DICTIONARY:
			printf("<\n");
			n = ((Dictionary *)pObj)->GetSize();
			for (i = 0; i < n; i++)
			{
				pName = ((Dictionary *)pObj)->GetName(i);
				printf(" %s: ", pName);
				Print(((Dictionary *)pObj)->GetValue(pName));
				printf("\n");
			}
			printf(">");
			break;
		case Object::OBJ_STREAM:
			Print(((Stream *)pObj)->GetDictionary());
			printf("\nstream size=%d", ((Stream *)pObj)->GetSize());
			break;
		case Object::OBJ_REFERENCE:
			printf("%d %d R", ((Reference *)pObj)->GetObjNum(), ((Reference *)pObj)->GetGeneration());
			break;
		case Object::OBJ_OPERATOR:
			printf("%s", ((Operator *)pObj)->GetValue());
			break;
		default:
			assert(false);
			break;
	}
}

Null::Null() : Object(OBJ_NULL)
{
}

Boolean::Boolean() : Object(OBJ_BOOLEAN)
{
	m_bValue = false;
}

void Boolean::SetValue(bool bValue)
{
	m_bValue = bValue;
}

bool Boolean::GetValue(void)
{
	return m_bValue;
}

Numeric::Numeric() : Object(OBJ_NUMERIC)
{
	m_dValue = 0.0;
}

void Numeric::SetValue(double dValue)
{
	m_dValue = dValue;
}

double Numeric::GetValue(void)
{
	return m_dValue;
}

String::String() : Object(OBJ_STRING)
{
	m_pValue = NULL;
	m_nLength = 0;
}

String::~String()
{
	delete[] m_pValue;
}

void String::SetValue(const char *pValue, int nLength, StringFormatType nFormat)
{
	const char *pSrc;
	char *pBuffer, *pDst, c;
	bool bBackSlash;
	int nOct;
	iconv_t cd;
	char *pFrom, *pTo;
	size_t nFrom, nTo;

	pSrc = pValue;
	pBuffer = new char[nLength + 1];
	pDst = pBuffer;
	if (nFormat == LITERAL)  //literal string
	{
		bBackSlash = false;
		nOct = 0;
		while (pSrc < pValue + nLength)
		{
			c = *pSrc++;
			if (bBackSlash)
			{
				bBackSlash = false;
				switch (c)
				{
					case 'n':
						c = '\n';
						break;
					case 'r':
						c = '\r';
						break;
					case 't':
						c = '\t';
						break;
					case 'b':
						c = '\b';
						break;
					case 'f':
						c = '\f';
						break;
					case '\\':
					case '(':
					case ')':
					case '[':
					case ']':
						break;
					case '\n':
					case '\r':
						continue;
					default:  //octal character code
						assert(c >= '0' && c <= '8');
						c -= '0';
						nOct = 2;
						break;
				}
				*pDst++ = c;
			}
			else if (c == '\\')
				bBackSlash = true;
			else
			{
				if (nOct > 0)
				{
					if (c >= '0' && c <= '8')
					{
						*(pDst - 1) = (*(pDst - 1) << 3) | (c - '0');
						--nOct;
					}
					else
						nOct = 0;
				}
				else
					*pDst++ = c;
			}
		}  //while
	}
	else if (nFormat == HEXADECIMAL)  //hexadecimal
	{
		while (pSrc < pValue + nLength)
		{
			*pDst++ = HtoD(*pSrc++) << 4;
			if (pSrc == pValue + nLength)
				break;
			*(pDst - 1) |= HtoD(*pSrc++);
		}
	}
	*pDst = '\0';

	if ((unsigned char)pBuffer[0] == 0xFE && (unsigned char)pBuffer[1] == 0xFF)
	{
		m_pValue = new char[(pDst - pBuffer) * 2 + 1];
		cd = iconv_open("UTF-8", "UTF-16BE");
		pFrom = pBuffer + 2;
		nFrom = pDst - pBuffer - 2;
		pTo = m_pValue;
		nTo = (pDst - pBuffer) * 2 + 1;
		iconv(cd, &pFrom, &nFrom, &pTo, &nTo);
		iconv_close(cd);
		*pTo = '\0';
		delete[] pBuffer;
		m_nLength = strlen(m_pValue);
	}
	else
	{
		m_pValue = pBuffer;
		m_nLength = pDst - pBuffer;
	}
}

const char *String::GetValue(void)
{
	return m_pValue;
}

int String::GetLength(void)
{
	return m_nLength;
}

Name::Name() : Object(OBJ_NAME)
{
	m_pValue = NULL;
}

Name::~Name()
{
	delete[] m_pValue;
}

void Name::SetValue(const char *pValue)
{
	char *ptr;

	m_pValue = new char[strlen(pValue) + 1];
	ptr = m_pValue;
	while (*pValue != '\0')
	{
		if (*pValue == '#')
		{
			*ptr++ = (HtoD(pValue[1]) << 4) | HtoD(pValue[2]);
			pValue += 3;
		}
		else
			*ptr++ = *pValue++;
	}
	*ptr = '\0';
}

const char *Name::GetValue(void)
{
	return m_pValue;
}

Array::Array() : Object(OBJ_ARRAY)
{
}

Array::~Array()
{
	vector<Object *>::iterator it;

	for (it = m_pValue.begin(); it != m_pValue.end(); ++it)
		delete *it;
}

void Array::Add(Object *pValue)
{
	Object *pObj[2];

	if (pValue->GetType() == OBJ_REFERENCE)
	{
		pObj[1] = m_pValue.back();
		m_pValue.pop_back();
		pObj[0] = m_pValue.back();
		m_pValue.pop_back();
		assert(pObj[0]->GetType() == OBJ_NUMERIC || pObj[1]->GetType() == OBJ_NUMERIC);
		((Reference *)pValue)->SetValue(((Numeric *)pObj[0])->GetValue(), ((Numeric *)pObj[1])->GetValue());
		delete pObj[0];
		delete pObj[1];
	}
	m_pValue.push_back(pValue);
}

int Array::GetSize(void)
{
	return m_pValue.size();
}

Object *Array::GetValue(int nIndex)
{
	return m_pValue.at(nIndex);
}

Dictionary::Dictionary() : Object(OBJ_DICTIONARY)
{
}

Dictionary::~Dictionary()
{
	vector<Object *>::iterator it;

	for (it = m_pName.begin(); it != m_pName.end(); ++it)
		delete *it;
	for (it = m_pValue.begin(); it != m_pValue.end(); ++it)
		delete *it;
}

void Dictionary::Add(Object *pName, Object *pValue)
{
	Object *pObj;

	if (pName->GetType() == OBJ_NAME)
	{
		m_pName.push_back(pName);
		m_pValue.push_back(pValue);
	}
	else
	{
		assert(pName->GetType() == OBJ_NUMERIC);
		assert(pValue->GetType() == OBJ_REFERENCE);
		pObj = m_pValue.back();
		m_pValue.pop_back();
		assert(pObj->GetType() == OBJ_NUMERIC);
		((Reference *)pValue)->SetValue(((Numeric *)pObj)->GetValue(), ((Numeric *)pName)->GetValue());
		m_pValue.push_back(pValue);
		delete pObj;
		delete pName;
	}
}

int Dictionary::GetSize(void)
{
	return m_pName.size();
}

const char *Dictionary::GetName(int nIndex)
{
	return ((Name *)m_pName.at(nIndex))->GetValue();
}

Object *Dictionary::GetValue(const char *pKey)
{
	int i;
	vector<Object *>::iterator it;

	i = 0;
	for (it = m_pName.begin(); it != m_pName.end(); ++it)
		if (strcmp(((Name *)*it)->GetValue(), pKey) == 0)
			break;
		else
			++i;
	return i < m_pValue.size()? m_pValue.at(i) : NULL;
}

Stream::Stream(Dictionary *pDict) : Object(OBJ_STREAM)
{
	m_pDict = pDict;
	m_pValue = NULL;
	m_nSize = 0;
}

Stream::~Stream()
{
	delete m_pDict;
	delete[] m_pValue;
}

Dictionary *Stream::GetDictionary()
{
	return m_pDict;
}

void Stream::SetValue(const unsigned char *pValue, int nSize)
{
	m_pValue = new unsigned char[nSize];
	memcpy(m_pValue, pValue, nSize);
	m_nSize = nSize;
}

const unsigned char *Stream::GetValue(void)
{
	return m_pValue;
}

int Stream::GetSize(void)
{
	return m_nSize;
}

Reference::Reference() : Object(OBJ_REFERENCE)
{
	m_nObjNum = -1;
	m_nGeneration = -1;
	m_pObj = NULL;
}

Reference::~Reference()
{
	delete m_pObj;
}

void Reference::SetValue(int nObjNum, int nGeneration)
{
	m_nObjNum = nObjNum;
	m_nGeneration = nGeneration;
}

int Reference::GetObjNum(void)
{
	return m_nObjNum;
}

int Reference::GetGeneration(void)
{
	return m_nGeneration;
}

void Reference::SetObject(Object *pObj)
{
	m_pObj = pObj;
}

Object *Reference::GetObject(void)
{
	return m_pObj;
}

Operator::Operator() : Object(OBJ_OPERATOR)
{
	m_pValue = NULL;
}

Operator::~Operator()
{
	delete[] m_pValue;
}

void Operator::SetValue(const char *pValue)
{
	m_pValue = new char[strlen(pValue) + 1];
	strcpy(m_pValue, pValue);
}

const char *Operator::GetValue(void)
{
	return m_pValue;
}
