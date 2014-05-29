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

CObject::CObject(ObjType_t nType)
{
	m_nType = nType;
	m_nOffset = -1;
}

CObject::~CObject()
{
}

CObject::ObjType_t CObject::GetType(void)
{
	return m_nType;
}

void CObject::SetOffset(unsigned int nOffset)
{
	m_nOffset = nOffset;
}

unsigned int CObject::GetOffset(void)
{
	return m_nOffset;
}

void CObject::Print(CObject *pObj)
{
	double d;
	int i, n;
	const char *pName;

	switch (pObj->GetType())
	{
		case CObject::OBJ_NULL:
			printf("null");
			break;
		case CObject::OBJ_BOOLEAN:
			printf("%s", ((CBoolean *)pObj)->GetValue()? "true" : "false");
			break;
		case CObject::OBJ_NUMERIC:
			d = ((CNumeric *)pObj)->GetValue();
			if (d == (int)d)
				printf("%d", (int)d);
			else
				printf("%lf", d);
			break;
		case CObject::OBJ_STRING:
			printf("\"%s\"", ((CString *)pObj)->GetValue());
			break;
		case CObject::OBJ_NAME:
			printf("/%s", ((CName *)pObj)->GetValue());
			break;
		case CObject::OBJ_ARRAY:
			printf("[");
			n = ((CArray *)pObj)->GetSize();
			for (i = 0; i < n; i++)
			{
				if (i > 0)
					printf(" ");
				Print(((CArray *)pObj)->GetValue(i));
			}
			printf("]");
			break;
		case CObject::OBJ_DICTIONARY:
			printf("<\n");
			n = ((CDictionary *)pObj)->GetSize();
			for (i = 0; i < n; i++)
			{
				pName = ((CDictionary *)pObj)->GetName(i);
				printf(" %s: ", pName);
				Print(((CDictionary *)pObj)->GetValue(pName));
				printf("\n");
			}
			printf(">");
			break;
		case CObject::OBJ_STREAM:
			Print(((CStream *)pObj)->GetDictionary());
			printf("\nstream size=%d", ((CStream *)pObj)->GetSize());
			break;
		case CObject::OBJ_REFERENCE:
			printf("%d %d R", ((CReference *)pObj)->GetObjNum(), ((CReference *)pObj)->GetGeneration());
			break;
		case CObject::OBJ_OPERATOR:
			printf("%s", ((COperator *)pObj)->GetValue());
			break;
		default:
			assert(false);
			break;
	}
}

CNull::CNull() : CObject(OBJ_NULL)
{
}

CBoolean::CBoolean() : CObject(OBJ_BOOLEAN)
{
	m_bValue = false;
}

void CBoolean::SetValue(bool bValue)
{
	m_bValue = bValue;
}

bool CBoolean::GetValue(void)
{
	return m_bValue;
}

CNumeric::CNumeric() : CObject(OBJ_NUMERIC)
{
	m_dValue = 0.0;
}

void CNumeric::SetValue(double dValue)
{
	m_dValue = dValue;
}

double CNumeric::GetValue(void)
{
	return m_dValue;
}

CString::CString() : CObject(OBJ_STRING)
{
	m_pValue = NULL;
	m_nLength = 0;
}

CString::~CString()
{
	delete[] m_pValue;
}

void CString::SetValue(const char *pValue, int nLength, StringFormat_t nFormat)
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

const char *CString::GetValue(void)
{
	return m_pValue;
}

int CString::GetLength(void)
{
	return m_nLength;
}

CName::CName() : CObject(OBJ_NAME)
{
	m_pValue = NULL;
}

CName::~CName()
{
	delete[] m_pValue;
}

void CName::SetValue(const char *pValue)
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

const char *CName::GetValue(void)
{
	return m_pValue;
}

CArray::CArray() : CObject(OBJ_ARRAY)
{
}

CArray::~CArray()
{
	vector<CObject *>::iterator it;

	for (it = m_pValue.begin(); it != m_pValue.end(); ++it)
		delete *it;
}

void CArray::Add(CObject *pValue)
{
	CObject *pObj[2];

	if (pValue->GetType() == OBJ_REFERENCE)
	{
		pObj[1] = m_pValue.back();
		m_pValue.pop_back();
		pObj[0] = m_pValue.back();
		m_pValue.pop_back();
		assert(pObj[0]->GetType() == OBJ_NUMERIC || pObj[1]->GetType() == OBJ_NUMERIC);
		((CReference *)pValue)->SetValue(((CNumeric *)pObj[0])->GetValue(), ((CNumeric *)pObj[1])->GetValue());
		delete pObj[0];
		delete pObj[1];
	}
	m_pValue.push_back(pValue);
}

int CArray::GetSize(void)
{
	return m_pValue.size();
}

CObject *CArray::GetValue(int nIndex)
{
	return m_pValue.at(nIndex);
}

CDictionary::CDictionary() : CObject(OBJ_DICTIONARY)
{
}

CDictionary::~CDictionary()
{
	vector<CObject *>::iterator it;

	for (it = m_pName.begin(); it != m_pName.end(); ++it)
		delete *it;
	for (it = m_pValue.begin(); it != m_pValue.end(); ++it)
		delete *it;
}

void CDictionary::Add(CObject *pName, CObject *pValue)
{
	CObject *pObj;

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
		((CReference *)pValue)->SetValue(((CNumeric *)pObj)->GetValue(), ((CNumeric *)pName)->GetValue());
		m_pValue.push_back(pValue);
		delete pObj;
		delete pName;
	}
}

int CDictionary::GetSize(void)
{
	return m_pName.size();
}

const char *CDictionary::GetName(int nIndex)
{
	return ((CName *)m_pName.at(nIndex))->GetValue();
}

CObject *CDictionary::GetValue(const char *pKey)
{
	int i;
	vector<CObject *>::iterator it;

	i = 0;
	for (it = m_pName.begin(); it != m_pName.end(); ++it)
		if (strcmp(((CName *)*it)->GetValue(), pKey) == 0)
			break;
		else
			++i;
	return i < m_pValue.size()? m_pValue.at(i) : NULL;
}

CStream::CStream(CDictionary *pDict) : CObject(OBJ_STREAM)
{
	m_pDict = pDict;
	m_pValue = NULL;
	m_nSize = 0;
}

CStream::~CStream()
{
	delete m_pDict;
	delete[] m_pValue;
}

CDictionary *CStream::GetDictionary()
{
	return m_pDict;
}

void CStream::SetValue(const unsigned char *pValue, int nSize)
{
	m_pValue = new unsigned char[nSize];
	memcpy(m_pValue, pValue, nSize);
	m_nSize = nSize;
}

const unsigned char *CStream::GetValue(void)
{
	return m_pValue;
}

int CStream::GetSize(void)
{
	return m_nSize;
}

CReference::CReference() : CObject(OBJ_REFERENCE)
{
	m_nObjNum = -1;
	m_nGeneration = -1;
	m_pObj = NULL;
}

CReference::~CReference()
{
	delete m_pObj;
}

void CReference::SetValue(int nObjNum, int nGeneration)
{
	m_nObjNum = nObjNum;
	m_nGeneration = nGeneration;
}

int CReference::GetObjNum(void)
{
	return m_nObjNum;
}

int CReference::GetGeneration(void)
{
	return m_nGeneration;
}

void CReference::SetObject(CObject *pObj)
{
	m_pObj = pObj;
}

CObject *CReference::GetObject(void)
{
	return m_pObj;
}

COperator::COperator() : CObject(OBJ_OPERATOR)
{
	m_pValue = NULL;
}

COperator::~COperator()
{
	delete[] m_pValue;
}

void COperator::SetValue(const char *pValue)
{
	m_pValue = new char[strlen(pValue) + 1];
	strcpy(m_pValue, pValue);
}

const char *COperator::GetValue(void)
{
	return m_pValue;
}
