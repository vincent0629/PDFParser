#include "DataInputStream.h"
#include <stdio.h>
#include <stdlib.h>

static bool IsSpace(int c)
{
	//return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f' || c == '\0';
	return c >= 0 && c <= ' ';
}

static bool IsDelimiter(int c)
{
	return c == '(' || c == ')' || c == '<' || c == '>' || c == '[' || c == ']' || c == '{' || c == '}' || c == '/' || c == '%';
}

DataInputStream::DataInputStream(InputStream *pSource)
{
	m_pSource = pSource;
}

int DataInputStream::ReadStr(char *pBuffer, int nSize)
{
	char *ptr;
	int c, nRet;
	bool bBackSlash;

	nRet = 0;
	ptr = pBuffer;
	bBackSlash = false;
	while (true)
	{
		c = m_pSource->Read();
		if (IsSpace(c))
		{
			if (!bBackSlash)
				break;
		}
		else if (IsDelimiter(c))
		{
			if (!bBackSlash)
			{
				m_pSource->Seek(-1, SEEK_CUR);
				break;
			}
		}
		else if (c == '\\')
			bBackSlash = !bBackSlash;
		else if (c == EOF)
			break;
		if (ptr < pBuffer + nSize - 1)
			*ptr++ = c;
		++nRet;
	}
	*ptr = '\0';

	Skip();

	return nRet;
}

int DataInputStream::ReadInt(void)
{
	char pBuffer[16], *ptr;
	int c;

	ptr = pBuffer;
	while (true)
	{
		c = m_pSource->Read();
		if ((c >= '0' && c <= '9') || c == '-' || c == '+')
			*ptr++ = c;
		else
		{
			m_pSource->Seek(-1, SEEK_CUR);
			break;
		}
	}
	*ptr = '\0';

	Skip();

	return atoi(pBuffer);
}

void DataInputStream::Skip(void)
{
	int c;

	do
	{
		c = m_pSource->Read();
	} while (IsSpace(c));

	if (c == '%')  //comment
	{
		do
		{
			c = m_pSource->Read();
		} while (c != '\n' && c != '\r' && c != EOF);
		do
		{
			c = m_pSource->Read();
		} while (c == '\n' || c == '\r');
	}

	if (c != EOF)
		m_pSource->Seek(-1, SEEK_CUR);
}
