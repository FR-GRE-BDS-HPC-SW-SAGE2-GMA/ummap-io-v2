/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_GMOCK_MAPPING_HPP
#define UMMAP_GMOCK_MAPPING_HPP

/********************  HEADERS  *********************/
//gtest
#include <gmock/gmock.h>
//internal
#include "Mapping.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * A mock of mapping object the be used for unite tests.
**/
class GMockMapping : public Mapping
{
	public:
		GMockMapping(void * addr, size_t size, size_t segmentSize, size_t storageOffset, int protection, int flags, Driver * driver, Policy * localPolicy = NULL, Policy * globalPolicy = NULL)
			: Mapping(addr, size, segmentSize, storageOffset, protection, flags, driver, localPolicy, globalPolicy) {};
		MOCK_METHOD(void, evict, (Policy * sourcePolicy, size_t segmentId));
};

}

#endif //UMMAP_GMOCK_MAPPING_HPP
