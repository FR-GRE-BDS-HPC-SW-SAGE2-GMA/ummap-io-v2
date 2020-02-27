/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstring>
#include <ctime>
#include <cassert>
//internal
#include "../common/Debug.hpp"
#include "../portability/OS.hpp"
//local
#include "Mapping.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
Mapping::Mapping(size_t size, size_t segmentSize, MappingProtection protection, Driver * driver, Policy * localPolicy, Policy * globalPolicy)
{
	//checks
	assume(size > 0, "Do not accept null size mapping");
	assume(segmentSize > 0, "Do not accept null segment size");
	assumeArg(size % UMMAP_PAGE_SIZE == 0, "Size should be multiple of page size (%1), got '%2'").arg(UMMAP_PAGE_SIZE).arg(size).end();
	assumeArg(size % segmentSize == 0, "Size should be multiple of segment size (%1), got '%2'").arg(segmentSize).arg(size).end();

	//set
	this->driver = driver->dup();
	this->localPolicy = localPolicy;
	this->globalPolicy = globalPolicy;
	this->protection = protection;

	//establish mapping
	this->baseAddress = OS::mmapProtNone(size);
	this->segments = size / segmentSize;
	this->segmentSize = segmentSize;

	//establish state tracking
	this->status = new SegmentStatus[this->segments];
	memset(this->status, 0, sizeof(SegmentStatus) * this->segments);
	for (size_t i = 0 ; i < this->segments ; i++)
		this->status[i].needRead = true;

	//build policy status local storage
	if (localPolicy != NULL)
		this->localPolicyStorage = localPolicy->getElementStorage(this, this->segments);
	else
		this->localPolicyStorage = NULL;
	
	//build policy status global storage
	if (globalPolicy != NULL)
		this->globalPolicyStorage = globalPolicy->getElementStorage(this, this->segments);
	else
		this->globalPolicyStorage = NULL;

	//build mutexes to protect segments
	this->segmentMutexesCnt = (OS::cpuNumber() * 4) + 1;
	if (this->segmentMutexesCnt > this->segments)
		this->segmentMutexesCnt = this->segments;
	this->segmentMutexes = new std::mutex[this->segmentMutexesCnt];
}

/*******************  FUNCTION  *********************/
Mapping::~Mapping(void)
{
	//take all locks
	for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
		this->segmentMutexes[i].lock();

	//destroy driver dup()
	delete this->driver;

	//policies
	if (this->localPolicyStorage != NULL && this->localPolicy != NULL)
		this->localPolicy->freeElementStorage(this->localPolicyStorage, this->segments);
	if (this->globalPolicyStorage != NULL && this->globalPolicy != NULL)
		this->globalPolicy->freeElementStorage(this->globalPolicyStorage, this->segments);

	//destroy all
	delete [] this->segmentMutexes;
}

/*******************  FUNCTION  *********************/
void * Mapping::getAddress(void)
{
	return this->baseAddress;
}

/*******************  FUNCTION  *********************/
void Mapping::loadAndSwapSegment(size_t offset, bool writeAccess)
{
	//In order to be atomic we load the data in a segment, then
	//we mremap this segment on the expected one so it replace atomicaly
	//the old one and open access. This is to avoid multi-threading issue
	//if a second thread made first touch while the first one is reading the
	//data if we just made madvise on the pre-existing PROT_NONE segment.

	//map a page in RW access
	void * ptr = OS::mmapProtFull(this->segmentSize);

	//read inside new segment
	ssize_t res = this->driver->pread(ptr, this->segmentSize, offset);
	assumeArg(res == segmentSize, "Fail to read all data, got %1 instead of %2 !")
		.arg(res)
		.arg(segmentSize)
		.end();

	//make read only
	if (!writeAccess)
		OS::mprotect(ptr, segmentSize, true, false);
	
	//now remap to move the segment and override the PROT_NONE one
	OS::mremapForced(ptr, segmentSize, (char*)this->baseAddress + offset);
}

/*******************  FUNCTION  *********************/
void Mapping::onSegmentationFault(void * address, bool isWrite)
{
	//checks
	assume(address >= this->baseAddress 
		&& address < (char*)this->baseAddress + this->segments * this->segmentSize,
		"Invalid address, not fit into the current segment !");

	//compute
	size_t segmentId = ((char*)address - (char*)this->baseAddress) / this->segmentSize;
	size_t offset = this->segmentSize * segmentId;
	void * segmentBase = (char*)this->baseAddress + offset;
	
	//CRITICAL SECTION
	{
		//lock to access
		int mutexId = segmentId % this->segmentMutexesCnt;
		std::lock_guard<std::mutex> lockGuard(this->segmentMutexes[mutexId]);

		//check status
		SegmentStatus & status = this->status[segmentId];

		//if not mapped
		if (!status.mapped){
			if (status.needRead) {
				//Load in a temp buffer and swap for atomicity
				this->loadAndSwapSegment(offset, isWrite);
			}
		}

		//if write or not
		if (isWrite) {
			//this is a write, open write access
			OS::mprotect(segmentBase, segmentSize, true, true);

			//mark dirty
			status.dirty = true;

			//time
			status.time = time(NULL);
		} else {
			//this is a first touch withou need read, open as readonly
			OS::mprotect(segmentBase, segmentSize, true, false);
		}

		//mark as mapped
		status.mapped = true;
	}

	//notify eviction policy
	if (this->localPolicy != NULL)
		this->localPolicy->touch(this->localPolicyStorage, offset, isWrite);
	if (this->globalPolicy != NULL)
		this->globalPolicy->touch(this->globalPolicyStorage, offset, isWrite);
}

/*******************  FUNCTION  *********************/
SegmentStatus Mapping::getSegmentStatus(size_t offset)
{
	//checks
	assert(offset < this->getSize());

	//compute
	size_t segmentId = offset / this->segmentSize;

	//CRITICAL SECTION
	{
		//lock to access
		int mutexId = segmentId % this->segmentMutexesCnt;
		std::lock_guard<std::mutex> lockGuard(this->segmentMutexes[mutexId]);

		//check status
		SegmentStatus & status = this->status[segmentId];

		//ok
		return status;
	}
}

/*******************  FUNCTION  *********************/
size_t Mapping::getSize(void) const
{
	return this->segmentSize * this->segments;
}

/*******************  FUNCTION  *********************/
void Mapping::flush(void)
{
	this->flush(0, getSize());
}

/*******************  FUNCTION  *********************/
void Mapping::flush(size_t offset, size_t size)
{
	//check
	assume(offset % segmentSize == 0, "Should get offset multiple of segment size !");
	assume(size % segmentSize == 0, "Should get offset multiple of segment size !");

	//TODO
}

/*******************  FUNCTION  *********************/
void Mapping::prefetch(size_t offset, size_t size)
{

}

/*******************  FUNCTION  *********************/
void Mapping::evict(size_t segmentId)
{

}

/*******************  FUNCTION  *********************/
void Mapping::skipFirstRead(void)
{
	for (size_t i = 0 ; i < this->segmentSize ; i++)
		this->status[i].needRead = false;
}
