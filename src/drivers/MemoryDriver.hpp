/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_MEMORY_DRIVER_HPP
#define UMMAP_MEMORY_DRIVER_HPP

/********************  HEADERS  *********************/
//internal
#include "../core/Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * Implement a memory driver. It is more a proof of concept for testing
 * ummap as it just allocate a memory space to store the data in addition
 * of the ummap mapping. It used memcpy() to synchronize the data between
 * this memory space and the mapping.
**/
class MemoryDriver : public Driver
{
	public:
		MemoryDriver(size_t size, char defaultValue = 0);
		MemoryDriver(MemoryDriver * driver);
		virtual ~MemoryDriver(void);
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void * ptr, size_t offset, size_t size) override;
		char * getBuffer(void);
		size_t getSize(void) const;
	private:
		/** Size of the memory space. **/
		size_t size;
		/** Pointer to the memory space. **/
		char * buffer;
		/** 
		 * Number of pointers to the buffer to be shared between multiple driver
		 * instances.
		**/
		int * share;
};

}

#endif //UMMAP_MEMORY_DRIVER_HPP
