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
#include "CDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
CDriver::CDriver(const ummap_c_driver_t * c_driver, void * driver_data)
{
	this->c_driver = *c_driver;
	this->driver_data = driver_data;
}

/*******************  FUNCTION  *********************/
CDriver::~CDriver(void)
{
	this->c_driver.finalize(this->driver_data);
}

/*******************  FUNCTION  *********************/
ssize_t CDriver::pwrite(const void * buffer, size_t size, size_t offset)
{
	return this->c_driver.pwrite(this->driver_data, buffer, size, offset);
}

/*******************  FUNCTION  *********************/
ssize_t CDriver::pread(void * buffer, size_t size, size_t offset)
{
	this->c_driver.pread(this->driver_data, buffer, size, offset);
}

/*******************  FUNCTION  *********************/
bool CDriver::directMSync(void * base, size_t size, size_t offset)
{
	if (this->c_driver.direct_msync == NULL)
		return false;
	else
		return this->c_driver.direct_msync(this->driver_data, base, size, offset);
}

/*******************  FUNCTION  *********************/
void CDriver::sync(void * ptr, size_t offset, size_t size)
{
	this->c_driver.pread(this->driver_data, ptr, offset, size);
}

/*******************  FUNCTION  *********************/
void * CDriver::directMmap(size_t size, size_t offset, bool read, bool write, bool exec)
{
	if (this->c_driver.direct_mmap == NULL)
		return false;
	else
		return this->c_driver.direct_mmap(this->driver_data, size, offset, read, write, exec);
}

/*******************  FUNCTION  *********************/
bool CDriver::directMunmap(void * base, size_t size, size_t offset)
{
	if (this->c_driver.direct_munmap == NULL)
		return false;
	else
		return this->c_driver.direct_munmap(this->driver_data, base, size, offset);
}
