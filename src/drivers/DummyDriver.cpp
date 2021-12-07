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
#include "DummyDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Constructor of the dummy driver.
 * @param value Define the valid to give to memset() to initiliaze the memory
 * on the first access.
**/
DummyDriver::DummyDriver(char value)
{
	this->value = value;
}

/*******************  FUNCTION  *********************/
DummyDriver::~DummyDriver(void)
{
}

/*******************  FUNCTION  *********************/
ssize_t DummyDriver::pwrite(const void * buffer, size_t size, size_t offset)
{
	//check
	assert(buffer != NULL);

	return size;
}

/*******************  FUNCTION  *********************/
ssize_t DummyDriver::pread(void * buffer, size_t size, size_t offset)
{
	//check
	assert(buffer != NULL);

	memset(buffer, this->value, size);
	return size;
}

/*******************  FUNCTION  *********************/
void DummyDriver::sync(void * ptr, size_t offset, size_t size)
{
	//nothing to do
}
