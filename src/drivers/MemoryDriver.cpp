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
	assume(buffer != NULL, "Invalid NULL buffer !");
	assume(offset < this->size, "Invalid offset, larger than memory size !");
	assume(size + offset < this->size, "Invalid offset and size, larger than memory size !");

	//copy
	memcpy(this->buffer + offset, buffer, size);

	//return
	return size;
}

/*******************  FUNCTION  *********************/
ssize_t MemoryDriver::pread(void * buffer, size_t size, size_t offset)
{
	//check
	assume(buffer != NULL, "Invalid NULL buffer !");
	assume(offset < this->size, "Invalid offset, larger than memory size !");
	assume(size + offset < this->size, "Invalid offset and size, larger than memory size !");

	//copy
	memcpy(buffer, this->buffer + offset, size);

	//return
	return size;
}

/*******************  FUNCTION  *********************/
void MemoryDriver::sync(size_t offset, size_t size)
{
	//nothing to do
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
