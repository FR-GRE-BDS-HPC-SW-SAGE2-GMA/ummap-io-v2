/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_MEMORY_DRIVER_HPP
#define UMMAP_MEMORY_DRIVER_HPP

/********************  HEADERS  *********************/
//internal
#include "../core/Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
class MemoryDriver : public Driver
{
	public:
		MemoryDriver(size_t size, char defaultValue = 0);
		virtual ~MemoryDriver(void);
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset);
		virtual ssize_t pread(void * buffer, size_t size, size_t offset);
		virtual void sync(size_t offset, size_t size);
		virtual Driver * dup(void);
		const char * getBuffer(void) const;
		size_t getSize(void) const;
	private:
		size_t size;
		char * buffer;
};

}

#endif //UMMAP_MEMORY_DRIVER_HPP
