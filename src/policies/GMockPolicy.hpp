/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_GMOCK_POLICY_HPP
#define UMMAP_GMOCK_POLICY_HPP

/********************  HEADERS  *********************/
//gtest
#include <gmock/gmock.h>
//internal
#include "../core/Policy.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
class GMockPolicy : public Policy
{
	public:
		GMockPolicy(void): Policy(2*4096, true) {};
		virtual ~GMockPolicy(void) {};
		MOCK_METHOD(void, allocateElementStorage,(Mapping * mapping, size_t segmentCount));
		MOCK_METHOD(void, notifyTouch,(Mapping * mapping, size_t index, bool isWrite));
		MOCK_METHOD(void, notifyEvict,(Mapping * mapping, size_t index));
		MOCK_METHOD(void, freeElementStorage,(Mapping * mapping));
};

}

#endif //UMMAP_GMOCK_POLICY_HPP
