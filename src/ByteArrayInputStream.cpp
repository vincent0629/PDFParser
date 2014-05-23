#include "ByteArrayInputStream.h"
#include <stdio.h>
#include <string.h>

CByteArrayInputStream::CByteArrayInputStream(const unsigned char *pData, int nSize)
{
	m_pData = pData;
	m_nSize = nSize;
	m_ptr = m_pData;
}

unsigned int CByteArrayInputStream::Available(void)
{
	return m_pData + m_nSize - m_ptr;
}

unsigned int CByteArrayInputStream::Position(void)
{
	return m_ptr - m_pData;
}

int CByteArrayInputStream::Seek(int nOffset, int nFrom)
{
	const unsigned char *ptr;

	ptr = m_ptr;
	if (nFrom == SEEK_CUR)
		ptr += nOffset;
	else if (nFrom == SEEK_SET)
		ptr = m_pData + nOffset;
	else if (nFrom == SEEK_END)
		ptr = m_pData + m_nSize + nOffset;
	if (ptr < m_pData)
		return -1;
	m_ptr = ptr;
	return 0;
}

int CByteArrayInputStream::Read(void)
{
	return m_ptr >= m_pData + m_nSize? EOF : *m_ptr++;
}

int CByteArrayInputStream::Read(void *pBuffer, int nSize)
{
	if (nSize > m_pData + m_nSize - m_ptr)
		nSize = m_pData + m_nSize - m_ptr;
	memcpy(pBuffer, m_ptr, nSize);
	m_ptr += nSize;
	return nSize;
}
