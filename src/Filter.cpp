#include "Filter.h"

Filter::Filter(InputStream *pSource)
{
	m_pSource = pSource;
}

Filter::~Filter()
{
	delete m_pSource;
}
