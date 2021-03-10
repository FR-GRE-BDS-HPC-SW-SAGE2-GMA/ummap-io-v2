/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_MAPPING_HPP
#define UMMAP_MAPPING_HPP

/********************  HEADERS  *********************/
//config
#include "config.h"
//std
#include <cstdlib>
#include <mutex>
//unix
#include <sys/mman.h>
//htopml
#ifdef HAVE_HTOPML
#include <htopml/JsonState.h>
#endif
//internal
#include "Policy.hpp"
#include "Driver.hpp"
#include "public-api/ummap.h"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  DEFINES  ********************/
/** Default value for flags on flush operation. **/
#define UMMAP_FLUSH_DEFAULT 0
/** Apply a sync operation after flushing the data. **/
#define UMMAP_FLUSH_SYNC 1
/** Unmap the memory after the flush operation. **/
#define UMMAP_FLUSH_UNMAP 2
/** Do not take locks when flushing data. **/
#define UMMAP_FLUSH_NO_LOCK 4

/*********************  STRUCT  *********************/
/**
 * Struct defining the status variable to track the stage of each segment of a mapping.
**/
struct SegmentStatus
{
	/** Last write access. **/
	size_t time:56;
	/** Unused **/
	unsigned int unused:5;
	/** True if the segment is mapped **/
	bool mapped:1;
	/** 
	 * True if the segment has been touched by a write access and is out of 
	 * sync with the storage. 
	**/
	bool dirty:1;
	/**
	 * If true the segment will be read on the first read access. If not it will be filled
	 * by zeroes on the first access.
	**/
	bool needRead:1;
};

/*********************  CLASS  **********************/
/**
 * Define a memory mapping and handle its state by calling the driver to 
 * read/write data from the storage. A mapping is also decribed by a policy
 * object to control its memory consumption.
**/
class Mapping
{
	public:
		Mapping(void * addr, size_t size, size_t segmentSize, size_t storageOffset, int protection, int flags, Driver * driver, Policy * localPolicy = NULL, Policy * globalPolicy = NULL);
		virtual ~Mapping(void);
		void onSegmentationFault(void * address, bool isWrite);
		void flush(bool sync);
		void flush(size_t offset, size_t size, int flags = UMMAP_FLUSH_DEFAULT);
		void prefetch(size_t offset, size_t size);
		virtual void evict(Policy * sourcePolicy, size_t segmentId);
		void * getAddress(void);
		void skipFirstRead(void);
		SegmentStatus getSegmentStatus(size_t offset);
		size_t getSize(void) const;
		size_t getAlignedSize(void) const;
		size_t getSegmentSize(void) const;
		void disableThreadSafety();
		Driver * getDriver(void);
		void unregisterRange(void);
		void registerRange(void);
		void dropClean(void);
	public:
		#ifdef HAVE_HTOPML
		friend void convertToJson(htopml::JsonState & json,const Mapping & value);
		#endif
	private:
		void loadAndSwapSegment(size_t offset, bool writeAccess);
		const bool * getMutexRange(size_t offset, size_t size, bool * buffer, size_t bufferSize) const;
		size_t readWriteSize(size_t offset);
	private:
		/** Driver to access the storage and read/write data from it. **/
		Driver * driver;
		/** Define the local policy to control the memory used by the current mapping. **/
		Policy * localPolicy;
		/** 
		 * Define a pointer to a global policy handling the memory consumption of severval
		 * segments.
		**/
		Policy * globalPolicy;
		/**
		 * Keep track of the protection flags to be applied. Allow flags are PROT_READ, PROT_WRITE, 
		 * PROT_EXEC as for the system mmap.
		**/
		int protection;
		/** Base address of the mapping. **/
		char * baseAddress;
		/** Number of segments composing the mapping **/
		size_t segments;
		/** Define the size of the segment composing the mapping **/
		size_t segmentSize;
		/** 
		 * Define the size of data to be mapped from the storage (the mapping can be a bit larger
		 * to fit multiplicity of segmentSize).
		**/
		size_t size;
		/** Define the offset in the storage. No necessily aligned on segmentSize. **/
		size_t storageOffset;
		/** Table of segment status.**/
		SegmentStatus * segmentStatus;
		/** Keep track of driver ID to identify the mapping. **/
		int64_t mappingDriverId;
		/** 
		 * Table of mutex to protect the various segment. This avoid to contention on a uniq
		 * mutex.
		**/
		std::mutex * segmentMutexes;
		/**
		 * Number of segment mutex in use. The choice is derived from the number of CPU threads
		 * observed when spawning the segment.
		**/
		int segmentMutexesCnt;
		/**
		 * Enable of disable the thread safety concerning the mprotect/mremap operations. (locks
		 * are kept).
		**/
		bool threadSafe;
};

/*******************  FUNCTION  *********************/
#ifdef HAVE_HTOPML
void convertToJson(htopml::JsonState & json,const SegmentStatus & value);
void convertToJson(htopml::JsonState & json,const Mapping & value);
#endif //HAVE_HTOPML

}

#endif //UMMAP_MAPPING_HPP
