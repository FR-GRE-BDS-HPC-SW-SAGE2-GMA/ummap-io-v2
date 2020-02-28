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
#include "DummyDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
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
void DummyDriver::sync(size_t offset, size_t size)
{
	//nothing to do
}

/*******************  FUNCTION  *********************/
Driver * DummyDriver::dup(void)
{
	return new DummyDriver(this->value);
}
