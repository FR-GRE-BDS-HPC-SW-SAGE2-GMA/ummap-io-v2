/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_FD_DRIVER_HPP
#define UMMAP_FD_DRIVER_HPP

/********************  HEADERS  *********************/
#include "../core/Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummap_io
{

/*********************  CLASS  **********************/
class FDDriver : public Driver
{
	public:
		FDDriver(int fd);
		~FDDriver(void);
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset);
		virtual ssize_t pread(void * buffer, size_t size, size_t offset);
		virtual void sync(size_t offset, size_t size);
	private:
		int fd;
};

}

#endif //UMMAP_FD_DRIVER_HPP
