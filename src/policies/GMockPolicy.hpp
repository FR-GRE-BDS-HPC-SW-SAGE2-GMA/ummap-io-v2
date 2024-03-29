/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_GMOCK_POLICY_HPP
#define UMMAP_GMOCK_POLICY_HPP

/********************  HEADERS  *********************/
//gtest
#include <gmock/gmock.h>
//internal
#include "../core/Policy.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * Provide a mocked policy to unit testing.
**/
class GMockPolicy : public Policy
{
	public:
		GMockPolicy(void): Policy(64*4096, true) {};
		virtual ~GMockPolicy(void) {};
		MOCK_METHOD(void, allocateElementStorage,(Mapping * mapping, size_t segmentCount), (override));
		MOCK_METHOD(void, notifyTouch,(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty), (override));
		MOCK_METHOD(void, notifyEvict,(Mapping * mapping, size_t index), (override));
		MOCK_METHOD(void, freeElementStorage,(Mapping * mapping), (override));
		MOCK_METHOD(size_t, getCurrentMemory,(), (override));
		MOCK_METHOD(void, shrinkMemory,(), (override));
};

}

#endif //UMMAP_GMOCK_POLICY_HPP
