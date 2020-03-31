/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstring>
#include <cassert>
//internal
#include "common/Debug.hpp"
#include "MemoryDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
MemoryDriver::MemoryDriver(size_t size, char defaultValue)
{
	//check
	assert(size > 0);
	this->size = size;
	this->buffer = new char[size];
	this->share = new int;
	*this->share = 1;
	memset(this->buffer, defaultValue, size);
}

/*******************  FUNCTION  *********************/
MemoryDriver::MemoryDriver(MemoryDriver * driver)
{
	//check
	assert(driver != NULL);
	assert(driver->share != NULL);

	//copy
	this->share = driver->share;
	this->buffer = driver->buffer;

	//inc sharing
	(*this->share)++;
}

/*******************  FUNCTION  *********************/
MemoryDriver::~MemoryDriver(void)
{
	if (*this->share == 1) {
		delete [] this->buffer;
		delete this->share;
	} else {
		(*this->share)--;
	}

	this->buffer = NULL;
	this->share = NULL;
}

/*******************  FUNCTION  *********************/
ssize_t MemoryDriver::pwrite(const void * buffer, size_t size, size_t offset)
{
	//check
	assert(buffer != NULL);
	assert(offset < this->size);
	assert(size + offset <= this->size);

	//copy
	memcpy(this->buffer + offset, buffer, size);

	//return
	return size;
}

/*******************  FUNCTION  *********************/
ssize_t MemoryDriver::pread(void * buffer, size_t size, size_t offset)
{
	//check
	assert(buffer != NULL);
	assert(offset < this->size);
	assert(size + offset <= this->size);

	//copy
	memcpy(buffer, this->buffer + offset, size);

	//return
	return size;
}

/*******************  FUNCTION  *********************/
void MemoryDriver::sync(size_t offset, size_t size)
{
	//check
	assert(buffer != NULL);
	assert(offset < this->size);
	assert(size + offset <= this->size);
}

/*******************  FUNCTION  *********************/
char * MemoryDriver::getBuffer(void)
{
	return this->buffer;
}

/*******************  FUNCTION  *********************/
size_t MemoryDriver::getSize(void) const
{
	return this->size;
}