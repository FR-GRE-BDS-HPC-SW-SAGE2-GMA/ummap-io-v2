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
