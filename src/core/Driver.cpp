/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include "Driver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap_io;

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
