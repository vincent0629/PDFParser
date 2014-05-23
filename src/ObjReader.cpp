#include "ObjReader.h"
#include "DataInputStream.h"
#include "Object.h"
#include "Xref.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CObjReader::CObjReader(CDataInputStream *pSource, CXref *pXref)
{
	m_pSource = pSource;
	m_pXref = pXref;
}

CObject *CObjReader::ReadObj(void)
{
	unsigned int nOffset, nOffTmp;
	int c, n;
	CObject *pObj, *pObj2;
	CDictionary *pDict;
	char str[32 * 1024], *ptr;
	int nParentheses;

	nOffset = m_pSource->Position();
	c = m_pSource->Read();
	switch (c)
	{
		case '/':  //name
			pObj = new CName();
			m_pSource->ReadStr(str, sizeof(str));
			((CName *)pObj)->SetValue(str);
			break;
		case '(':  //string
			pObj = new CString();
			ptr = str;
			nParentheses = 0;
			while (true)
			{
				c = m_pSource->Read();
				if (c == '\\')
				{
					*ptr++ = c;
					c = m_pSource->Read();
				}
				else if (c == '(')
					++nParentheses;
				else if (c == ')')
				{
					if (--nParentheses <= 0)
						break;
				}
				*ptr++ = c;
			}
			*ptr = '\0';
			((CString *)pObj)->SetValue(str, ptr - str, CString::LITERAL);
			break;
		case '<':
			c = m_pSource->Read();
			if (c == '<')  //dictionary
			{
				pDict = new CDictionary();
				while (true)
				{
					m_pSource->Skip();
					c = m_pSource->Read();
					if (c == '>')
					{
						m_pSource->Read();  //>
						m_pSource->Skip();
						m_pSource->Read(str, 6);
						if (strncmp(str, "stream", 6) == 0)  //stream
						{
							pObj = pDict->GetValue("Length");
							if (pObj->GetType() == CObject::OBJ_REFERENCE)
							{
								nOffTmp = m_pSource->Position();
								pObj = ReadIndirectObj(((CReference *)pObj)->GetObjNum(), ((CReference *)pObj)->GetGeneration());
								m_pSource->Seek(nOffTmp, SEEK_SET);
								n = ((CNumeric *)pObj)->GetValue();
								delete pObj;
							}
							else
								n = ((CNumeric *)pObj)->GetValue();
							pObj = new CStream(pDict);
							m_pSource->Skip();
							ptr = new char[n];
							m_pSource->Read(ptr, n);
							((CStream *)pObj)->SetValue((unsigned char *)ptr, n);
							delete[] ptr;
						}
						else
						{
							pObj = pDict;
							m_pSource->Seek(-6, SEEK_CUR);
						}
						break;
					}
					else
					{
						m_pSource->Seek(-1, SEEK_CUR);
						pObj = ReadObj();
						pDict->Add(pObj, ReadObj());
					}
				}
			}
			else  //hexadecimal string
			{
				m_pSource->Seek(-1, SEEK_CUR);
				pObj = new CString();
				n = m_pSource->ReadStr(str, sizeof(str));
				((CString *)pObj)->SetValue(str, n, CString::HEXADECIMAL);
				m_pSource->Read();  //>
			}
			break;
		case '[':  //array
			pObj = new CArray();
			while (true)
			{
				m_pSource->Skip();
				c = m_pSource->Read();
				if (c == ']')
					break;
				m_pSource->Seek(-1, SEEK_CUR);
				((CArray *)pObj)->Add(ReadObj());
			}
			break;
		default:
			*str = c;
			m_pSource->ReadStr(str + 1, sizeof(str) - 1);
			if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.')
			{
				pObj = new CNumeric();
				((CNumeric *)pObj)->SetValue(atof(str));
			}
			else
			{
				if (strcmp(str, "R") == 0)
					pObj = new CReference();
				else if (strcmp(str, "true") == 0)
				{
					pObj = new CBoolean();
					((CBoolean *)pObj)->SetValue(true);
				}
				else if (strcmp(str, "false") == 0)
				{
					pObj = new CBoolean();
					((CBoolean *)pObj)->SetValue(false);
				}
				else if (strcmp(str, "null") == 0)
					pObj = new CNull();
				else
				{
					pObj = new COperator();
					((COperator *)pObj)->SetValue(str);
				}
			}
			break;
	}
	m_pSource->Skip();

	pObj->SetOffset(nOffset);
	return pObj;
}

CObject *CObjReader::ReadIndirectObj(int nObjNum, int nGeneration)
{
	unsigned int nOffset;
	char str[4];

	nOffset = m_pXref->GetOffset(nObjNum, nGeneration);
	if (nOffset == 0)
		return NULL;
	m_pSource->Seek(nOffset, SEEK_SET);
	m_pSource->ReadInt();  //object number
	m_pSource->ReadInt();  //generation
	m_pSource->ReadStr(str, sizeof(str));  //obj
if (nObjNum == 1589)
	nObjNum = 1589;
	return ReadObj();
}
