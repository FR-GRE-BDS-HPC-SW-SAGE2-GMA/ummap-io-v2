/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_IOC_DRIVER_HPP
#define UMMAP_IOC_DRIVER_HPP

/********************  HEADERS  *********************/
//internal
#include "../core/Driver.hpp"
#include <ioc-client.h>

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * Implement the driver for the io-catcher makeing a cache between ummap and
 * Mero.
**/
class IocDriver : public Driver
{
	public:
		IocDriver(ioc_client_t * client, int64_t high, int64_t low, bool create = true);
		virtual ~IocDriver(void) override;
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void * ptr, size_t offset, size_t size) override;
		virtual int64_t establish_mapping(size_t offset, size_t size, bool write) override;
		virtual void erase_mapping(int64_t data, size_t offset, size_t size, bool write) override;
		virtual bool checkThreadSafety(void) override;
		int cow(int64_t high, int64_t low, bool allowExist, size_t offset, size_t size);
		void switchDestination(int64_t high, int64_t low);
	private:
		/** Keep track of the client struct to communicate. **/
		ioc_client_t * client;
		/** High part of the object ID **/
		int64_t high;
		/** Low part of the object ID **/
		int64_t low;
};

}

#endif //UMMAP_IOC_DRIVER_HPP
