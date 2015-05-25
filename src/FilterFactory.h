#ifndef _FILTERFACTORY_H_
#define _FILTERFACTORY_H_

class Dictionary;
class InputStream;

class FilterFactory
{
public:
	static InputStream *Create(const char *pName, const Dictionary *pParms, InputStream *pSource);
};

#endif
