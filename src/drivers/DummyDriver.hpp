/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_DUMMY_DRIVER_HPP
#define UMMAP_DUMMY_DRIVER_HPP

/********************  HEADERS  *********************/
//internal
#include "../core/Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * The dummy driver implement a simple driver just copying a value into memory
 * on first access.
**/
class DummyDriver : public Driver
{
	public:
		DummyDriver(char value = 0);
		virtual ~DummyDriver(void) override;
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void * ptr, size_t offset, size_t size) override;
	private:
		/** Value to use in the memset() call on first access to the mapping. **/
		char value;
};

}

#endif //UMMAP_DUMMY_DRIVER_HPP
