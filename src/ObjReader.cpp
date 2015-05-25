#include "ObjReader.h"
#include "DataInputStream.h"
#include "Object.h"
#include "Xref.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ObjReader::ObjReader(DataInputStream *pSource, const Xref *pXref)
{
	m_pSource = pSource;
	m_pXref = pXref;
}

const Object *ObjReader::ReadObj(void)
{
	unsigned int nOffset, nOffTmp;
	int c, n;
	Object *pObj;
	Dictionary *pDict;
	const Object *pcObj;
	char str[32 * 1024], *ptr;
	int nParentheses;

	nOffset = m_pSource->Position();
	c = m_pSource->Read();
	switch (c)
	{
		case '/':  //name
			pObj = new Name();
			m_pSource->ReadStr(str, sizeof(str));
			((Name *)pObj)->SetValue(str);
			break;
		case '(':  //string
			pObj = new String();
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
			((String *)pObj)->SetValue(str, ptr - str, String::LITERAL);
			break;
		case '<':
			c = m_pSource->Read();
			if (c == '<')  //dictionary
			{
				pDict = new Dictionary();
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
							pcObj = pDict->GetValue("Length");
							if (pcObj->GetType() == Object::OBJ_REFERENCE)
							{
								nOffTmp = m_pSource->Position();
								pcObj = ReadIndirectObj(((const Reference *)pcObj)->GetObjNum(), ((const Reference *)pcObj)->GetGeneration());
								m_pSource->Seek(nOffTmp, SEEK_SET);
								n = ((const Numeric *)pcObj)->GetValue();
								delete pcObj;
							}
							else
								n = ((const Numeric *)pcObj)->GetValue();
							pObj = new Stream(pDict);
							m_pSource->Skip();
							ptr = new char[n];
							m_pSource->Read(ptr, n);
							((Stream *)pObj)->SetValue((unsigned char *)ptr, n);
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
						pcObj = ReadObj();
						pDict->Add(pcObj, ReadObj());
					}
				}
			}
			else  //hexadecimal string
			{
				m_pSource->Seek(-1, SEEK_CUR);
				pObj = new String();
				n = m_pSource->ReadStr(str, sizeof(str));
				((String *)pObj)->SetValue(str, n, String::HEXADECIMAL);
				m_pSource->Read();  //>
			}
			break;
		case '[':  //array
			pObj = new Array();
			while (true)
			{
				m_pSource->Skip();
				c = m_pSource->Read();
				if (c == ']')
					break;
				m_pSource->Seek(-1, SEEK_CUR);
				((Array *)pObj)->Add(ReadObj());
			}
			break;
		default:
			*str = c;
			m_pSource->ReadStr(str + 1, sizeof(str) - 1);
			if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.')
			{
				pObj = new Numeric();
				((Numeric *)pObj)->SetValue(atof(str));
			}
			else
			{
				if (strcmp(str, "R") == 0)
					pObj = new Reference();
				else if (strcmp(str, "true") == 0)
				{
					pObj = new Boolean();
					((Boolean *)pObj)->SetValue(true);
				}
				else if (strcmp(str, "false") == 0)
				{
					pObj = new Boolean();
					((Boolean *)pObj)->SetValue(false);
				}
				else if (strcmp(str, "null") == 0)
					pObj = new Null();
				else if (*str == ' ')  // is this normal?
					pObj = new Object();  // invalid object
				else
				{
					pObj = new Operator();
					((Operator *)pObj)->SetValue(str);
				}
			}
			break;
	}
	m_pSource->Skip();

	pObj->SetOffset(nOffset);
	return pObj;
}

const Object *ObjReader::ReadIndirectObj(int nObjNum, int nGeneration)
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
	return ReadObj();
}
