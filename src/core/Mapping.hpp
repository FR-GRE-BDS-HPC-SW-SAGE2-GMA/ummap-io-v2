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
#include <mutex>
//internal
#include "Policy.hpp"
#include "Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  TYPES  **********************/
enum MappingProtection
{
	MAPPING_PROT_NONE = 0,
	MAPPING_PROT_READ = 1,
	MAPPING_PROT_WRITE = 2,
	MAPPING_PROT_RW = 3,
};

/*********************  STRUCT  *********************/
struct SegmentStatus
{
	size_t time:56;
	unsigned int unused:5;
	bool mapped:1;
	bool dirty:1;
	bool needRead:1;
};

/*********************  CLASS  **********************/
class Mapping
{
	public:
		Mapping(size_t size, size_t segmentSize, size_t storageOffset, MappingProtection protection, Driver * driver, Policy * localPolicy = NULL, Policy * globalPolicy = NULL);
		virtual ~Mapping(void);
		void onSegmentationFault(void * address, bool isWrite);
		void sync(void);
		void sync(size_t offset, size_t size, bool unmap = false, bool lock = true);
		void prefetch(size_t offset, size_t size);
		virtual void evict(Policy * sourcePolicy, size_t segmentId);
		void * getAddress(void);
		void skipFirstRead(void);
		SegmentStatus getSegmentStatus(size_t offset);
		size_t getSize(void) const;
		size_t getAlignedSize(void) const;
		size_t getSegmentSize(void) const;
	private:
		void loadAndSwapSegment(size_t offset, bool writeAccess);
		const bool * getMutexRange(size_t offset, size_t size) const;
		size_t readWriteSize(size_t offset);
	private:
		Driver * driver;
		Policy * localPolicy;
		Policy * globalPolicy;
		MappingProtection protection;
		char * baseAddress;
		size_t segments;
		size_t segmentSize;
		size_t size;
		size_t storageOffset;
		SegmentStatus * segmentStatus;
		std::mutex * segmentMutexes;
		int segmentMutexesCnt;
};

}

#endif //UMMAP_MAPPING_HPP
