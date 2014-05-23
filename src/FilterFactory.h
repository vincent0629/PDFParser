#ifndef _FILTERFACTORY_H_
#define _FILTERFACTORY_H_

class CDictionary;
class IInputStream;

class CFilterFactory
{
public:
	static IInputStream *Create(const char *pName, CDictionary *pParms, IInputStream *pSource);
};

#endif
