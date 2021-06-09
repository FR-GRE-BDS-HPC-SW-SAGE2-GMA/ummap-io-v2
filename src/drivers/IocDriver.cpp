/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//linux
#include <sys/utsname.h>
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
	//pre first-touch
	for (size_t i = 0 ; i < size; i += 4096)
		((char*)buffer)[i] = 0;

	//read
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

/*******************  FUNCTION  *********************/
/**
 * Apply a copy on write operation on the current object handled by the driver and switched to it.
 * @param high The high part of the new object ID.
 * @param low The low part of the new object ID.
 * @param allowExist Allow to cow on a pre-existing object.
 * @param offset Base offset of the range to cow.
 * @param size Size of then range to cow.
 * @return 0 on success, negative value on error.
**/
int IocDriver::cow(int64_t high, int64_t low, bool allowExist, size_t offset, size_t size)
{
	//trivial
	if (this->high == high && this->low == low)
		return 0;
	
	//aooly cow
	int status = ioc_client_obj_cow(this->client, this->high, this->low, high, low, allowExist, offset, size);
	assumeArg(status == 0, "Failed to apply COW (%1:%2 -> %3:%4) [%5:%6] operation of the given driver !")
		.arg(this->high).arg(this->low)
		.arg(high).arg(low)
		.arg(offset).arg(size)
		.end();

	//remember new ID
	this->high = high;
	this->low = low;

	//ret
	return status;
}

/*******************  FUNCTION  *********************/
/**
 * Switch the current object ID without more actions.
 * @param high The high part of the new object ID.
 * @param low The low part of the new object ID.
**/
void IocDriver::switchDestination(int64_t high, int64_t low)
{
	ioc_client_obj_create(this->client, this->high, this->low);
	this->high = high;
	this->low = low;
}

/*******************  FUNCTION  *********************/
bool IocDriver::checkThreadSafety(void)
{
	//global static variables
	static bool gblChecked = false;
	static bool gblHasRecentKernel = false;

	//if need to check
	if (gblChecked == false) {
		//get os infos
		struct utsname buf;
		int status = uname(&buf);
		assume(status == 0, "Fail to call uname() to check OS version !");

		//extract
		int major, minor, release;
		status = sscanf(buf.release, "%d.%d.%d", &major, &minor, &release);
		assumeArg(status == 3, "Fail to parse kernel version: %1").arg(buf.release).end();

		//check
		gblHasRecentKernel = (major > 3);

		//abort not to crash the system
		if (!gblHasRecentKernel)
			UMMAP_FATAL("You are using 3.X kernel version. IOC is known to lead to kernel deadlock when using ummap-io thread safety semantic, please use UMMAP_THREAD_UNSAFE !");
		
		//mark checked
		gblChecked = true;
	}

	return gblHasRecentKernel;
}
