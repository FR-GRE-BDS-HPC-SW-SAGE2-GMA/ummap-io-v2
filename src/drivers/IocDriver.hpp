/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
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
class IocDriver : public Driver
{
	public:
		IocDriver(struct ioc_client_t * client, int64_t high, int64_t low);
		virtual ~IocDriver(void) override;
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void * ptr, size_t offset, size_t size) override;
	private:
		struct ioc_client_t * client;
		int64_t high;
		int64_t low;
};

}

#endif //UMMAP_IOC_DRIVER_HPP
