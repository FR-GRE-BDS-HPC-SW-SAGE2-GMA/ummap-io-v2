/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//internal
#include "IocDriver.hpp"

/********************  NAMESPACE  *******************/
using namespace ummapio;

/*********************  CLASS  **********************/
IocDriver::IocDriver(struct ioc_client_t * client, int64_t high, int64_t low)
{
	this->client = client;
	this->high = high;
	this->low = low;
}

/*******************  FUNCTION  *********************/
IocDriver::~IocDriver(void)
{
}

/*******************  FUNCTION  *********************/
ssize_t IocDriver::pwrite(const void * buffer, size_t size, size_t offset)
{
	return ioc_client_obj_write(this->client, this->high, this->low, buffer, size, offset);
}

/*******************  FUNCTION  *********************/
ssize_t IocDriver::pread(void * buffer, size_t size, size_t offset)
{
	return ioc_client_obj_read(this->client, this->high, this->low, buffer, size, offset);
}

/*******************  FUNCTION  *********************/
void IocDriver::sync(void * ptr, size_t offset, size_t size)
{
	ioc_client_obj_flush(this->client, this->high, this->low, offset, size);
}
