#include "CMapReader.h"
#include "CMap.h"
#include "DataInputStream.h"
#include "Object.h"
#include "ObjReader.h"
#include <stdio.h>
#include <string.h>

static unsigned int StringToCode(const String *pString)
{
	int i, nCode, nLen;
	const unsigned char *cstr;
	int code;

	nCode = 0;
	cstr = (const unsigned char *)pString->GetValue();
	nLen = pString->GetLength();
	for (i = 0; i < nLen; ++i)
		nCode = (nCode << 8) | cstr[i];
	return nCode;
}

CMapReader::CMapReader(CMap *pCMap)
{
	m_pCMap = pCMap;
}

void CMapReader::Read(InputStream *pSource)
{
	DataInputStream *pDIS;
	ObjReader *pReader;
	const Object *pObj;
	const Array *pArray;
	const char *cstr;
	unsigned int i, nFrom, nTo, nCode;

	pDIS = new DataInputStream(pSource);
	pReader = new ObjReader(pDIS, NULL);
	while (true)
	{
		pObj = pReader->ReadObj();
		if (pObj == NULL)
			break;

		if (pObj->GetType() == Object::OBJ_OPERATOR)
		{
			cstr = ((const Operator *)pObj)->GetValue();
			if (strcmp(cstr, "beginbfchar") == 0 || strcmp(cstr, "begincidchar") == 0)
			{
				while (true)
				{
					pObj = pReader->ReadObj();
					if (pObj->GetType() == Object::OBJ_OPERATOR)  // end
						break;
					nFrom = StringToCode((const String *)pObj);
					pObj = pReader->ReadObj();
					nCode = StringToCode((const String *)pObj);
					m_pCMap->Set(nFrom, nCode);
				}
			}
			else if (strcmp(cstr, "beginbfrange") == 0 || strcmp(cstr, "begincidrange") == 0)
			{
				while (true)
				{
					pObj = pReader->ReadObj();
					if (pObj->GetType() == Object::OBJ_OPERATOR)  // end
						break;
					nFrom = StringToCode((const String *)pObj);
					pObj = pReader->ReadObj();
					nTo = StringToCode((const String *)pObj);
					pObj = pReader->ReadObj();
					if (pObj->GetType() == Object::OBJ_STRING)
					{
						nCode = StringToCode((const String *)pObj);
						for (i = nFrom; i <= nTo; ++i)
							m_pCMap->Set(i, nCode + i - nFrom);
					}
					else
					{
						pArray = (const Array *)pObj;
						for (i = nFrom; i <= nTo; ++i)
						{
							pObj = pArray->GetValue(i - nFrom);
							m_pCMap->Set(i, StringToCode((const String *)pObj));
						}
					}
				}
			}
		}
	}
	delete pReader;
	delete pDIS;
}
