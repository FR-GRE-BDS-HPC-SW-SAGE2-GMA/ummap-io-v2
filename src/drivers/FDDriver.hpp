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
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * Implement a driver accessing the a file via its file descriptor.
**/
class FDDriver : public Driver
{
	public:
		FDDriver(int fd);
		~FDDriver(void);
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void * ptr, size_t offset, size_t size) override;
		void setFd(int fd);
		int getFd(void) {return fd;};
	private:
		/** File descriptor to be used for data transfers **/
		int fd;
};

}

#endif //UMMAP_FD_DRIVER_HPP
