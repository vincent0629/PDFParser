#include "Filter.h"

CFilter::CFilter(IInputStream *pSource)
{
	m_pSource = pSource;
}

CFilter::~CFilter()
{
	delete m_pSource;
}
