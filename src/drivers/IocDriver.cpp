/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//internal
#include "../common/Debug.hpp"
#include "IocDriver.hpp"

/********************  NAMESPACE  *******************/
using namespace ummapio;

/*********************  CLASS  **********************/
IocDriver::IocDriver(ioc_client_t * client, int64_t high, int64_t low, bool create)
{
	this->client = client;
	this->high = high;
	this->low = low;
	if (create)
		ioc_client_obj_create(this->client, this->high, this->low);
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

/*******************  FUNCTION  *********************/
int64_t IocDriver::establish_mapping(size_t offset, size_t size, bool write)
{
	int id = ioc_client_obj_range_register(this->client, this->high, this->low, offset, size, write);
	assume(id >= 0, "Fail to register range to establish IOC mapping, conflict with another segment already in using on this range in non compatible write mode.");
	return id;
}

/*******************  FUNCTION  *********************/
void IocDriver::erase_mapping(int64_t data, size_t offset, size_t size, bool write)
{
	int res = ioc_client_obj_range_unregister(this->client, (int32_t)data, this->high, this->low, offset, size, write);
	assume(res == 0, "Fail to unregister the given mapping on the IOC server !");
}
