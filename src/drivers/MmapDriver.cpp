/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstring>
#include <cassert>
//unix
#include <unistd.h>
#include <sys/mman.h>
//internal
#include "../portability/OS.hpp"
#include "../common/Debug.hpp"
#include "MmapDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
MmapDriver::MmapDriver(int fd, bool allowNotAligned)
{
	//setup
	this->allowNotAligned = allowNotAligned;

	//fd case
	if (fd == 0) {
		this->fd = 0;
	} else {
		int dupFD = ::dup(fd);
		assumeArg(dupFD > 0, "Fail to dup() the file descriptor : %1").argStrErrno().end();
		this->fd = dupFD;
	}
}

/*******************  FUNCTION  *********************/
MmapDriver::~MmapDriver(void)
{
}

/*******************  FUNCTION  *********************/
ssize_t MmapDriver::pwrite(const void * buffer, size_t size, size_t offset)
{
	//should not be called
	assert(false);
}

/*******************  FUNCTION  *********************/
ssize_t MmapDriver::pread(void * buffer, size_t size, size_t offset)
{
	//should not be called
	assert(false);
}

/*******************  FUNCTION  *********************/
void MmapDriver::sync(size_t offset, size_t size)
{
	//nothing to do
}

/*******************  FUNCTION  *********************/
void * MmapDriver::directMmap(size_t size, size_t offset, bool read, bool write)
{
	//alignement
	size_t addrOffset = 0;
	this->checkAndSetAlign(size, offset, addrOffset);

	//prot
	int prot = 0;
	if (read)
		prot |= PROT_READ;
	if (write)
		prot |= PROT_WRITE;

	//mmap
	void * res = NULL;
	if (this->fd == 0) {
		res = mmap(NULL, size, prot, MAP_ANON|MAP_PRIVATE, 0, 0);
	} else {
		res = mmap(NULL, size, prot, MAP_PRIVATE, fd, offset);
	}

	//check
	assumeArg(res != MAP_FAILED, "Fail to mmap : %1").argStrErrno().end();

	//return
	return (char*)res + addrOffset;
}

/*******************  FUNCTION  *********************/
bool MmapDriver::directMunmap(void * base, size_t size, size_t offset)
{
	//alignement
	size_t addrOffset = 0;
	this->checkAndSetAlign(size, offset, addrOffset);

	//unmap
	int res = munmap((char*)base - addrOffset, size);
	assumeArg(res == 0, "Fail to munmap : %1").argStrErrno().end();

	return true;
}

/*******************  FUNCTION  *********************/
void MmapDriver::checkAndSetAlign(size_t & size, size_t & offset, size_t & addrOffset)
{
	//check align
	if (allowNotAligned == false) {
		assumeArg(size % UMMAP_PAGE_SIZE == 0, "Size must be aligned to page size, got : %1 with alignemdnd : %2")
			.arg(size)
			.arg(size % UMMAP_PAGE_SIZE)
			.end();
		assumeArg(offset % UMMAP_PAGE_SIZE == 0, "Offset must be aligned to page size, got : %1 with alignemdnd : %2")
			.arg(offset)
			.arg(offset % UMMAP_PAGE_SIZE)
			.end();
	}

	//align offset
	if (offset % UMMAP_PAGE_SIZE != 0) {
		addrOffset = offset % UMMAP_PAGE_SIZE;
		offset -= addrOffset;
		size += addrOffset;
	}

	//align size
	if (size % UMMAP_PAGE_SIZE != 0)
		size += UMMAP_PAGE_SIZE - (size % UMMAP_PAGE_SIZE);
}
