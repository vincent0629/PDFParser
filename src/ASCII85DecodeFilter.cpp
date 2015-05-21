#include "ASCII85DecodeFilter.h"
#include "ByteArrayInputStream.h"

ASCII85DecodeFilter::ASCII85DecodeFilter(InputStream *pSource) : CompoundFilter(pSource)
{
	unsigned char *ptr;
	int i, j, c, n;

	m_pBuffer = new unsigned char[pSource->Available() * 4];
	ptr = m_pBuffer;
	while (true)
	{
		c = pSource->Read();
		if (c == '~')
			break;
		if (c <= ' ')
			continue;
		if (c == 'z')
		{
			n = 0;
			i = 4;
		}
		else
		{
			n = c - 33;
			i = 0;
			while (i < 4)
			{
				c = pSource->Read();
				if (c == '~')
					break;
				if (c > ' ')
				{
					n = n * 85 + (c - 33);
					++i;
				}
			}
			for (j = i; j < 4; j++)
				n *= 85;
		}
		for (j = 3; j >= 0; j--)
		{
			ptr[j] = n & 0xFF;
			n >>= 8;
		}
		ptr += i;
		if (i < 4)
			break;
	}
	m_pStream = new ByteArrayInputStream(m_pBuffer, ptr - m_pBuffer);
}

ASCII85DecodeFilter::~ASCII85DecodeFilter()
{
	delete m_pStream;
	delete[] m_pBuffer;
}
