/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_C_DRIVER_HPP
#define UMMAP_C_DRIVER_HPP

/********************  HEADERS  *********************/
//internal
#include "../core/Driver.hpp"
#include "../public-api/ummap.h"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * Wrapper to let the user implement a new driver directly from C language.
 * This wrapper mostly redirect the calls to overrided C++ functions to the C
 * pointed functions from the C struct.
**/
class CDriver : public Driver
{
	public:
		CDriver(const ummap_c_driver_t * c_driver, void * driver_data);
		virtual ~CDriver(void) override;
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void* ptr, size_t offset, size_t size) override;
		virtual void * directMmap(void * addr, size_t size, size_t offset, bool read, bool write, bool exec, bool mapFixed) override;
		virtual bool directMunmap(void * base, size_t size, size_t offset) override;
		virtual bool directMSync(void * base, size_t size, size_t offset) override;
	private:
		/** Structure containing the function pointer to implement the driver. **/
		ummap_c_driver_t c_driver;
		/** Pointer to raw data to transmit to the driver functions when called. **/
		void * driver_data;
};

}

#endif //UMMAP_C_DRIVER_HPP
