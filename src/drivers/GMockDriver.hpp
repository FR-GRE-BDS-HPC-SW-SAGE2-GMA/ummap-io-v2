/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_GMOCK_DRIVER_HPP
#define UMMAP_GMOCK_DRIVER_HPP

/********************  HEADERS  *********************/
//gtest
#include <gmock/gmock.h>
//internal
#include "../core/Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
class GMockDriver : public Driver
{
	public:
		GMockDriver(void) {};
		virtual ~GMockDriver(void) {};
		MOCK_METHOD(ssize_t, pwrite,(const void * buffer, size_t size, size_t offset));
		MOCK_METHOD(ssize_t, pread,(void * buffer, size_t size, size_t offset));
		MOCK_METHOD(void, sync,(size_t offset, size_t size));
	private:
		char value;
};

}

#endif //UMMAP_GMOCK_DRIVER_HPP
