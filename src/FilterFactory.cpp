#include "FilterFactory.h"
#include "ASCII85DecodeFilter.h"
#include "ASCIIHexDecodeFilter.h"
#include "Object.h"
#include "FlateDecodeFilter.h"
#include "PredictorFilter.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int GetParm(const Object *pDict, const char *pName, int nDefault)
{
	const Object *pObj;

	pObj = pDict? ((const Dictionary *)pDict)->GetValue(pName) : NULL;
	return pObj == NULL? nDefault : ((const Numeric *)pObj)->GetValue();
}

InputStream *FilterFactory::Create(const char *pName, const Dictionary *pParms, InputStream *pSource)
{
	int nPredictor, nColors, nBitsPerComponent, nColumns;

	if (strcmp(pName, "ASCIIHexDecode") == 0 || strcmp(pName, "AHx") == 0)
		pSource = new ASCIIHexDecodeFilter(pSource);
	else if (strcmp(pName, "ASCII85Decode") == 0 || strcmp(pName, "A85") == 0)
		pSource = new ASCII85DecodeFilter(pSource);
	else if (strcmp(pName, "LZWDecode") == 0 || strcmp(pName, "LZW") == 0)
		assert(false);
	else if (strcmp(pName, "FlateDecode") == 0 || strcmp(pName, "Fl") == 0)
	{
		pSource = new FlateDecodeFilter(pSource);
		nPredictor = GetParm(pParms, "Predictor", 1);
		if (nPredictor != 1)
		{
			nColors = GetParm(pParms, "Colors", 1);
			nBitsPerComponent = GetParm(pParms, "BitsPerComponent", 8);
			nColumns = GetParm(pParms, "Columns", 1);
			assert(false);
			pSource = new PredictorFilter(pSource, nColors, nBitsPerComponent, nColumns);
		}
	}
	else if (strcmp(pName, "RunLengthDecode") == 0 || strcmp(pName, "RL") == 0)
		assert(false);
	else if (strcmp(pName, "CCITTFaxDecode") == 0 || strcmp(pName, "CCF") == 0)
		assert(false);
	else if (strcmp(pName, "DCTDecode") == 0 || strcmp(pName, "DCT") == 0)
		assert(false);
	else
	{
		printf("%s\n", pName);
		assert(false);  //not implemented
	}
	return pSource;
}
