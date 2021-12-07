/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstring>
#include <cassert>
//internal
#include "common/Debug.hpp"
#include "MemoryDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Create a memory driver for the given size.
 * @param size Define the size of the memory space to be handled by the driver.
 * @param defaultValue Value to be transmitted to memset() to initilize the 
 * memory space.
**/
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
/**
 * Create a clone version of the memory driver sharing the memory space with
 * the given driver. In this case the last driver sharing will be resposnible
 * of the memory freeing.
 * @param driver Pointer to the original driver to share its buffer.
**/
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
	assumeArg(offset < this->size, "Invalid position, offset overpass memory limit: offset=%2, mem=%3")
		.arg(offset)
		.arg(this->size)
		.end();
	assumeArg(size + offset <= this->size, "Invalid position, overpass memory limit: size=%1, offset=%2, mem=%3")
		.arg(size)
		.arg(offset)
		.arg(this->size)
		.end();

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
	assumeArg(offset < this->size, "Invalid position, offset overpass memory limit: offset=%1, mem=%2")
		.arg(offset)
		.arg(this->size)
		.end();
	assumeArg(size + offset <= this->size, "Invalid position, overpass memory limit: size=%1, offset=%2, mem=%3")
		.arg(size)
		.arg(offset)
		.arg(this->size)
		.end();

	//copy
	memcpy(buffer, this->buffer + offset, size);

	//return
	return size;
}

/*******************  FUNCTION  *********************/
void MemoryDriver::sync(void * ptr, size_t offset, size_t size)
{
	//check
	assert(buffer != NULL);
	assert(offset < this->size);
	//assert(size + offset <= this->size);
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