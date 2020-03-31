/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_GMOCK_MAPPING_HPP
#define UMMAP_GMOCK_MAPPING_HPP

/********************  HEADERS  *********************/
//gtest
#include <gmock/gmock.h>
//internal
#include "Mapping.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
class GMockMapping : public Mapping
{
	public:
		GMockMapping(size_t size, size_t segmentSize, size_t storageOffset, MappingProtection protection, Driver * driver, Policy * localPolicy = NULL, Policy * globalPolicy = NULL)
			: Mapping(size, segmentSize, storageOffset, protection, driver, localPolicy, globalPolicy) {};
		MOCK_METHOD(void, evict, (Policy * sourcePolicy, size_t segmentId));
};

}

#endif //UMMAP_GMOCK_MAPPING_HPP
