#include "CMapReader.h"
#include "CMap.h"
#include "InputStream.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bool IsSpace(int c)
{
	//return c == ' ' || c == '\n' || c == '\r' || c == '\t';
	return c >= 0 && c <= ' ';
}

static const char *ReadToken(InputStream *pSource)
{
	static char str[80];
	char *ptr;
	int nState, c;

	ptr = str;
	nState = 1;
	while (nState > 0)
	{
		c = pSource->Read();
		if (c == EOF)
			break;

		switch (nState)
		{
			case 1:
				if (c == '%')
					nState = 3;
				else if (!IsSpace(c))
				{
					*ptr++ = c;
					nState = 2;
				}
				break;
			case 2:
				if (IsSpace(c))
					nState = 0;
				else if (c == '/')
				{
					pSource->Seek(-1, SEEK_CUR);
					nState = 0;
				}
				else
					*ptr++ = c;
				break;
			case 3:
				if (c == '\n' || c == '\r')
					nState = 4;
				break;
			case 4:
				if (c != '\n' || c != '\r')
				{
					pSource->Seek(-1, SEEK_CUR);
					nState = 1;
				}
				break;
		}
	}
	*ptr = '\0';

	return str;
}

CMapReader::CMapReader(CMap *pCMap)
{
	m_pCMap = pCMap;
	m_nCount = 0;
}

void CMapReader::Read(InputStream *pSource)
{
	const char *str;
	unsigned int i, nFrom, nTo, nCode;

	while (true)
	{
		str = ReadToken(pSource);
		if (str[0] == '\0')
			break;

		if (strcmp(str, "beginbfchar") == 0 || strcmp(str, "begincidchar") == 0)
		{
			while (true)
			{
				str = ReadToken(pSource);
				if (strncmp(str, "end", 3) == 0)
					break;
				nFrom = *str == '<'? strtol(str + 1, NULL, 16) : atoi(str);
				str = ReadToken(pSource);
				nCode = *str == '<'? strtol(str + 1, NULL, 16) : atoi(str);
				m_pCMap->Set(nFrom, nCode);
			}
		}
		else if (strcmp(str, "beginbfrange") == 0 || strcmp(str, "begincidrange") == 0)
		{
			while (true)
			{
				str = ReadToken(pSource);
				if (strncmp(str, "end", 3) == 0)
					break;
				nFrom = *str == '<'? strtol(str + 1, NULL, 16) : atoi(str);
				str = ReadToken(pSource);
				nTo = *str == '<'? strtol(str + 1, NULL, 16) : atoi(str);
				str = ReadToken(pSource);
				nCode = *str == '<'? strtol(str + 1, NULL, 16) : atoi(str);
				for (i = nFrom; i <= nTo; i++)
					m_pCMap->Set(i, nCode + i - nFrom);
			}
		}
	}

	++m_nCount;
}
