#include "ASCIIHexDecodeFilter.h"
#include "ByteArrayInputStream.h"

static int HtoD(int c)
{
	if (c >= '0' && c <= '9')
		c -= '0';
	else if (c >= 'A' && c <= 'F')
		c -= 'A' - 10;
	else if (c >= 'a' && c <= 'f')
		c -= 'a' - 10;
	else
		c = 0;
	return c;
}

CASCIIHexDecodeFilter::CASCIIHexDecodeFilter(IInputStream *pSource) : CCompoundFilter(pSource)
{
	unsigned char *ptr;
	int c, n;

	m_pBuffer = new unsigned char[pSource->Available() * 2];
	ptr = m_pBuffer;
	while (true)
	{
		c = pSource->Read();
		if (c == '>')
			break;
		if (c <= ' ')
			continue;
		n = HtoD(c) << 4;
		c = pSource->Read();
		n |= HtoD(c);
		*ptr++ = n;
		if (c == '>')
			break;
	}
	m_pStream = new CByteArrayInputStream(m_pBuffer, ptr - m_pBuffer);
}

CASCIIHexDecodeFilter::~CASCIIHexDecodeFilter()
{
	delete m_pStream;
	delete[] m_pBuffer;
}
