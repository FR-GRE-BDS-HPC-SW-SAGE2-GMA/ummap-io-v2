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
	memset(this->buffer, defaultValue, size);
}

/*******************  FUNCTION  *********************/
MemoryDriver::~MemoryDriver(void)
{
	delete [] this->buffer;
	this->buffer = NULL;
}

/*******************  FUNCTION  *********************/
ssize_t MemoryDriver::pwrite(const void * buffer, size_t size, size_t offset)
{
	//check
	assert(buffer != NULL);
	assert(offset < this->size);
	assert(size + offset <= this->size);

	//copy
	assert(((char*)buffer)[0] == 64);
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
Driver * MemoryDriver::dup(void)
{
	//alloc new
	MemoryDriver * copy = new MemoryDriver(this->size, 0);

	//copy content
	memcpy(copy->buffer, this->buffer, this->size);

	//return
	return copy;
}

/*******************  FUNCTION  *********************/
const char * MemoryDriver::getBuffer(void) const
{
	return this->buffer;
}

/*******************  FUNCTION  *********************/
size_t MemoryDriver::getSize(void) const
{
	return this->size;
}