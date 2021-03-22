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
#include <sys/types.h>
#include <sys/stat.h>
//internal
#include "../portability/OS.hpp"
#include "../common/Debug.hpp"
#include "MmapDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Constructir of the direct mmap driver.
 * @param fd File descriptor of the file to mmap. Can be 0 for anonymous memory.
 * @param allowNotAligned Define if we allow not aligned mappings (offset and size).
**/
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
	if (fd != 0)
		close(fd);
}

/*******************  FUNCTION  *********************/
ssize_t MmapDriver::pwrite(const void * buffer, size_t size, size_t offset)
{
	//should not be called
	assert(false);
	return size;
}

/*******************  FUNCTION  *********************/
ssize_t MmapDriver::pread(void * buffer, size_t size, size_t offset)
{
	//should not be called
	assert(false);
	return size;
}

/*******************  FUNCTION  *********************/
bool MmapDriver::directMSync(void * base, size_t size, size_t offset)
{
	//alignement
	size_t addrOffset = 0;
	this->checkAndSetAlign(size, offset, addrOffset);

	//apply
	int res = msync((char*)base - addrOffset, size, MS_SYNC);
	assumeArg(res == 0, "Fail to msync(%1, %2, MS_SYNC) : %3")
		.arg(base)
		.arg(offset)
		.argStrErrno()
		.end();
	
	//say to not run soft sync
	return true;
}

/*******************  FUNCTION  *********************/
void MmapDriver::sync(void * ptr, size_t offset, size_t size)
{
	//should not be called
	assert(false);
}

/*******************  FUNCTION  *********************/
void * MmapDriver::directMmap(void * addr, size_t size, size_t offset, bool read, bool write, bool exec, bool mapFixed)
{
	//alignement
	size_t addrOffset = 0;
	this->checkAndSetAlign(size, offset, addrOffset);

	//truncate if needed
	if (write) {
		//get file size
		struct stat st;
		int status = fstat(this->fd, &st);
		assumeArg(status == 0, "Fail to fstat the file before memory mapping it: %1").argStrErrno().end();
		size_t origSize = st.st_size;

		//if size is too small we extend
		if (origSize < offset + size){
			status = ftruncate(this->fd,  offset+size);
			//assumeArg(status == 0, "Fail to truncate the file brefore memory mapping it: %1").argStrErrno().end();
		}
	}

	//prot
	int prot = 0;
	if (read)
		prot |= PROT_READ;
	if (write)
		prot |= PROT_WRITE;
	if (write)
		prot |= PROT_EXEC;

	//extra flags
	int flags = 0;
	if (mapFixed)
		flags |= MAP_FIXED;

	//mmap
	void * res = NULL;
	if (this->fd == 0) {
		res = mmap(addr, size, prot, MAP_ANON | MAP_PRIVATE | flags, 0, 0);
	} else {
		res = mmap(addr, size, prot, MAP_FILE | MAP_SHARED | flags, fd, offset);
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
/**
 * Check the alignement restrictor and apply offset/size fixes to fit with
 * the page alignement constrain of the mapping.
 * @param size Take the mapping size and will be aligned on page size.
 * @param offset Take the offset in file and will be aligned on page size.
 * @param addrOffset Will be updated to emulate the not aligned mapping.
**/
void MmapDriver::checkAndSetAlign(size_t & size, size_t & offset, size_t & addrOffset)
{
	//check align
	if (allowNotAligned == false) {
		assumeArg(size % UMMAP_PAGE_SIZE == 0, "Size must be aligned to page size, got : %1 with alignement : %2")
			.arg(size)
			.arg(size % UMMAP_PAGE_SIZE)
			.end();
		assumeArg(offset % UMMAP_PAGE_SIZE == 0, "Offset must be aligned to page size, got : %1 with alignement : %2")
			.arg(offset)
			.arg(offset % UMMAP_PAGE_SIZE)
			.end();
	}

	//align offset
	if (offset % UMMAP_PAGE_SIZE != 0) {
		addrOffset = offset % UMMAP_PAGE_SIZE;
		offset -= addrOffset;
		size += addrOffset;
	} else {
		addrOffset = 0;
	}

	//align size
	if (size % UMMAP_PAGE_SIZE != 0)
		size += UMMAP_PAGE_SIZE - (size % UMMAP_PAGE_SIZE);
}

/*******************  FUNCTION  *********************/
/**
 * Used to change the file descriptor in use for copy on write actions.
 * @param fd The new file descriptor.
**/
void MmapDriver::setFd(int newFd)
{
	//check
	assert(newFd > 0);

	//close old
	close(this->fd);

	//dup and set
	int dupFD = ::dup(newFd);
	assumeArg(dupFD > 0, "Fail to dup() the file descriptor : %1").argStrErrno().end();
	this->fd = dupFD;
}
