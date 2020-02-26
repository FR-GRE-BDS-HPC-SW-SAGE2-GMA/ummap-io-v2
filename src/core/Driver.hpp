/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_DRIVER_HPP
#define UMMAP_DRIVER_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
class Driver
{
	public:
		Driver(void);
		virtual ~Driver(void);
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) = 0;
		virtual ssize_t pwrite(void * buffer, size_t size, size_t offset) = 0;
		virtual void sync(size_t offset, size_t size) = 0;
};

}

#endif //UMMAP_DRIVER_HPP
