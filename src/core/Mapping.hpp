/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_MAPPING_HPP
#define UMMAP_MAPPING_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>
//internal
#include "Policy.hpp"
#include "Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  STRUCT  *********************/
struct SegmentStatus
{
	size_t flushTime:56;
	unsigned int unused:6;
	bool readAccess:1;
	bool writeAccess:1;
};

/*********************  CLASS  **********************/
class Mapping
{
	public:
		Mapping(size_t size, size_t segmentSize, Driver * driver, Policy * localPolicy = NULL, Policy * globalPolicy = NULL);
		virtual ~Mapping(void);
		void onSegmentationFault(void * address, bool isWrite);
		void flush(void);
		void flush(size_t offset, size_t size);
		void prefetch(size_t offset, size_t size);
		void evict(size_t segmentId);
	private:
		Driver * driver;
		Policy * localPolicy;
		Policy * globalPolicy;
		void * baseAddress;
		size_t segments;
		SegmentStatus * status;
};

}

#endif //UMMAP_MAPPING_HPP
