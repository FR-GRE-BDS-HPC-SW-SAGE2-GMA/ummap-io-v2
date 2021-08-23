/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cassert>
#include <cstring>
#include <cerrno>
//unix
#include <unistd.h>
//OS dependent for clone
#include <linux/fs.h>        /* Definition of FICLONE* constants */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//local
#include "../common/Debug.hpp"
#include "FDDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Driver constructor with the file descriptor.
 * The constructor make a dup() on this file descriptor so the caller can
 * make a close on its descriptor.
**/
FDDriver::FDDriver(int fd)
{
	assert(fd > 0);
	int dupFD = ::dup(fd);
	assumeArg(dupFD > 0, "Fail to dup() the file descriptor : %1").argStrErrno().end();
	this->fd = dupFD;
}

/*******************  FUNCTION  *********************/
FDDriver::~FDDriver(void)
{
	close(fd);
}

/*******************  FUNCTION  *********************/
void FDDriver::setFd(int newFd)
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

/*******************  FUNCTION  *********************/
ssize_t FDDriver::pwrite(const void * buffer, size_t size, size_t offset)
{
	//checks
	assert(buffer != NULL);
	assert(size > 0);

	//apply
	return ::pwrite(fd, buffer, size, offset);
}

/*******************  FUNCTION  *********************/
ssize_t FDDriver::pread(void * buffer, size_t size, size_t offset)
{
	//checks
	assert(buffer != NULL);
	assert(size > 0);

	//apply
	return ::pread(fd, buffer, size, offset);
}

/*******************  FUNCTION  *********************/
void FDDriver::sync(void * ptr, size_t offset, size_t size)
{
	::fdatasync(fd);
}

/*******************  FUNCTION  *********************/
bool FDDriver::cloneRange(int targetFd, size_t offset, size_t size)
{
	//build info
	struct file_clone_range range = {
		this->fd,
		size,
		offset,
		offset
	};

	//call
	int status = ioctl(targetFd, FICLONERANGE, &range);

	//check ok
	return status == 0;
}
