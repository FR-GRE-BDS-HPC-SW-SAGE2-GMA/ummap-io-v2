/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
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
void Driver::setUri(const std::string & uri)
{
	this->uri = uri;
}

/*******************  FUNCTION  *********************/
const std::string & Driver::getUri(void) const
{
	return this->uri;
}
