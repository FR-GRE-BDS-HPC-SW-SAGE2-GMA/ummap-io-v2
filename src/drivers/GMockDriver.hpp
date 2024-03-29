/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_GMOCK_DRIVER_HPP
#define UMMAP_GMOCK_DRIVER_HPP

/********************  HEADERS  *********************/
//gtest
#include <gmock/gmock.h>
//internal
#include "../core/Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * Implement a mock driver based on GoogleMock for unit testing.
**/
class GMockDriver : public Driver
{
	public:
		GMockDriver(void) {};
		virtual ~GMockDriver(void) {};
		MOCK_METHOD(ssize_t, pwrite,(const void * buffer, size_t size, size_t offset), (override));
		MOCK_METHOD(ssize_t, pread,(void * buffer, size_t size, size_t offset), (override));
		MOCK_METHOD(void, sync,(void * ptr, size_t offset, size_t size), (override));
};

}

#endif //UMMAP_GMOCK_DRIVER_HPP
