#include "FlateDecodeFilter.h"
#include "ByteArrayInputStream.h"
#include <string.h>
#include <zlib.h>

FlateDecodeFilter::FlateDecodeFilter(InputStream *pSource) : CompoundFilter(pSource)
{
	unsigned char *pBuffer;
	unsigned long nSrcSize, nSize;
	z_stream zs;

	nSrcSize = pSource->Available();
	pBuffer = new unsigned char[nSrcSize];
	pSource->Read(pBuffer, nSrcSize);
	nSize = nSrcSize * 20;
	m_pBuffer = new unsigned char[nSize];
	memset(&zs, 0, sizeof(z_stream));
	zs.avail_in = nSrcSize;
	zs.avail_out = nSize;
	zs.next_in = pBuffer;
	zs.next_out = m_pBuffer;
	inflateInit(&zs);
	inflate(&zs, Z_FINISH);
	nSize = zs.total_out;
	inflateEnd(&zs);
	delete[] pBuffer;

	m_pStream = new ByteArrayInputStream(m_pBuffer, nSize);
}

FlateDecodeFilter::~FlateDecodeFilter()
{
	delete m_pStream;
	delete[] m_pBuffer;
}
