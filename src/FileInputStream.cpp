#include "FileInputStream.h"
#include "ByteArrayInputStream.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

FileInputStream::FileInputStream(const char *pFile)
{
	struct stat data;

	m_fd = open(pFile, O_RDONLY);
	if (m_fd != -1)
	{
		fstat(m_fd, &data);
		m_nSize = data.st_size;
		m_pData = (unsigned char *)mmap(NULL, m_nSize, PROT_READ, MAP_SHARED, m_fd, 0);  //create mapping
	}
	else
	{
		m_nSize = 0;
		m_pData = NULL;
	}
	m_pSource = new ByteArrayInputStream(m_pData, m_nSize);
}

FileInputStream::~FileInputStream()
{
	delete m_pSource;
	if (m_fd != -1)
	{
		munmap(m_pData, m_nSize);
		close(m_fd);
	}
}
