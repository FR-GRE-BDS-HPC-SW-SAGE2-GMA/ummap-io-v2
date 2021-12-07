/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
#include "Driver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Base constructor, do nothing for now.
**/
Driver::Driver(void)
{
	this->autoclean = false;
}

/*******************  FUNCTION  *********************/
/**
 * Base destructor, do nothing for now.
**/
Driver::~Driver(void)
{
}

/*******************  FUNCTION  *********************/
/**
 * Change the autoclean status (by default need to be enabled).
 * If enabled the driver will automatically be destroyed
 * by the umunmap() operation.
 * @param status Define the new value.
**/
void Driver::setAutoclean(bool status)
{
	this->autoclean = status;
}

/*******************  FUNCTION  *********************/
/**
 * @return Return the current autoclean status to be called
 * by Mapping at unmap time.
**/
bool Driver::hasAutoclean(void) const
{
	return this->autoclean;
}

/*******************  FUNCTION  *********************/
/**
 * Setup the URI of the driver to be printed by htopml if enabled.
 * @param uri Define the uri value to assign.
**/
void Driver::setUri(const std::string & uri)
{
	this->uri = uri;
}

/*******************  FUNCTION  *********************/
/**
 * Return the current URI as a string.
**/
const std::string & Driver::getUri(void) const
{
	return this->uri;
}

/*******************  FUNCTION  *********************/
void * Driver::directMmap(void *addr, size_t size, size_t offset, bool read, bool write, bool exec, bool mapFixed)
{
	return NULL;
}

/*******************  FUNCTION  *********************/
bool Driver::directMunmap(void * base, size_t size, size_t offset)
{
	return false;
}

/*******************  FUNCTION  *********************/
bool Driver::directMSync(void * base, size_t size, size_t offset)
{
	return false;
}

/*******************  FUNCTION  *********************/
int64_t Driver::establish_mapping(size_t offset, size_t size, bool write)
{
	return 0;
}

/*******************  FUNCTION  *********************/
void Driver::erase_mapping(int64_t data, size_t offset, size_t size, bool write)
{
	//nothing to do
}

/*******************  FUNCTION  *********************/
bool Driver::checkThreadSafety(void)
{
	return true;
}
