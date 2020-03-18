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
	assumeArg(segmentSize % UMMAP_PAGE_SIZE == 0, "Segment size should be multiple of page size (%1), got '%2'").arg(UMMAP_PAGE_SIZE).arg(size).end();
	assumeArg(size % UMMAP_PAGE_SIZE == 0, "Size should be multiple of page size (%1), got '%2'").arg(UMMAP_PAGE_SIZE).arg(size).end();
	assumeArg(size % segmentSize == 0, "Size should be multiple of segment size (%1), got '%2'").arg(segmentSize).arg(size).end();

	//set
	this->driver = driver->dup();
	this->localPolicy = localPolicy;
	this->globalPolicy = globalPolicy;
	this->protection = protection;

	//establish mapping
	this->baseAddress = (char*)OS::mmapProtNone(size);
	this->segments = size / segmentSize;
	this->segmentSize = segmentSize;

	//establish state tracking
	this->segmentStatus = new SegmentStatus[this->segments];
	memset(this->segmentStatus, 0, sizeof(SegmentStatus) * this->segments);
	for (size_t i = 0 ; i < this->segments ; i++)
		this->segmentStatus[i].needRead = true;

	//build policy status local storage
	if (localPolicy != NULL)
		localPolicy->allocateElementStorage(this, this->segments);
	
	//build policy status global storage
	if (globalPolicy != NULL)
		globalPolicy->allocateElementStorage(this, this->segments);

	//build mutexes to protect segments
	//
	//Rational 1: we use X mutexes so spread the contention. The access code will use
	//a modulo to select the right mutex based on the segment ID.
	//
	//Rational 2: The +1 is used to break possible aligned accesses between two threads
	//with distance exactly modulo. With an odd number there is less chance to fall in
	//those multiples
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
	if (this->localPolicy != NULL)
		this->localPolicy->freeElementStorage(this);
	if (this->globalPolicy != NULL)
		this->globalPolicy->freeElementStorage(this);
	
	//clear policy
	if (this->localPolicy != NULL)
		delete this->localPolicy;

	//destroy all mutexes
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
	//Rational: In order to be atomic we load the data in a segment, then
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
	OS::mremapForced(ptr, segmentSize, this->baseAddress + offset);
}

/*******************  FUNCTION  *********************/
void Mapping::onSegmentationFault(void * address, bool isWrite)
{
	//checks
	assume(address >= this->baseAddress 
		&& address < this->baseAddress + this->segments * this->segmentSize,
		"Invalid address, not fit into the current segment !");

	//compute
	size_t segmentId = ((char*)address - this->baseAddress) / this->segmentSize;
	size_t offset = this->segmentSize * segmentId;
	void * segmentBase = this->baseAddress + offset;
	
	//CRITICAL SECTION
	{
		//lock to access
		int mutexId = segmentId % this->segmentMutexesCnt;
		std::lock_guard<std::mutex> lockGuard(this->segmentMutexes[mutexId]);

		//check status
		SegmentStatus & status = this->segmentStatus[segmentId];

		//already done
		if (isWrite == status.dirty && status.mapped)
			return;

		//if not mapped
		if (!status.mapped && status.needRead){
			//Load in a temp buffer and swap for atomicity
			this->loadAndSwapSegment(offset, isWrite);
		}

		//if write or not
		if (isWrite) {
			//this is a write, open write access
			OS::mprotect(segmentBase, segmentSize, true, true);

			//mark dirty
			status.dirty = true;

			//update dirty time for latter flush operation
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
		this->localPolicy->notifyTouch(this, offset, isWrite);
	if (this->globalPolicy != NULL)
		this->globalPolicy->notifyTouch(this, offset, isWrite);
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
		SegmentStatus & status = this->segmentStatus[segmentId];

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
const bool * Mapping::getMutexRange(size_t offset, size_t size) const
{
	//check
	assert(offset < this->getSize());
	assert(offset % this->segmentSize == 0);
	assert(offset + size <= this->getSize());
	assert(size % this->segmentSize == 0);

	//allocate
	bool * list = new bool[this->segmentMutexesCnt];
	for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
		list[i] = false;

	//range
	size_t baseId = offset / this->segmentSize;
	size_t cnt = size / this->segmentSize;

	//loop
	for (size_t id = baseId ; id < baseId + cnt ; id++)
		list[id % this->segmentMutexesCnt] = true;

	//final
	return list;
}

/*******************  FUNCTION  *********************/
void Mapping::flush(void)
{
	this->flush(0, getSize());
}

/*******************  FUNCTION  *********************/
void Mapping::flush(size_t offset, size_t size, bool unmap)
{
	//check
	assume(offset < this->getSize(), "Offset is not in valid range !");
	assume(offset + size <= this->getSize(), "'Offset + size' is not in valid range !");
	assume(offset % segmentSize == 0, "Should get offset multiple of segment size !");
	assume(size % segmentSize == 0, "Should get offset multiple of segment size !");

	//what to lock
	const bool * toLock = getMutexRange(offset, size);

	//CRITICAL SECTION
	{
		//lock the whole segment
		//@TODO Can search to lock only related pages
		for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
			if (toLock[i])
				this->segmentMutexes[i % this->segmentMutexesCnt].lock();

		//mprotect the whole considered segment
		OS::mprotect(this->baseAddress + offset, size, true, false);

		//loop on all
		//@TODO: bulk operation
		for (size_t curOffset = offset ; curOffset < offset + size ; curOffset += this->segmentSize) {
			size_t id = curOffset / this->segmentSize;
			//check status
			SegmentStatus & status = this->segmentStatus[id];
			if (status.dirty) {
				//compute
				void * segmentPtr = this->baseAddress + curOffset;

				//apply
				ssize_t res = this->driver->pwrite(segmentPtr, this->segmentSize, curOffset);

				//errors
				assumeArg(res != -1, "Fail to pwrite : %1").arg(strerror(errno)).end();
				assumeArg(res == this->segmentSize, "Fail to fully write the segment, got : %1").arg(res).end();

				//update status
				status.dirty = false;
				status.needRead = true;
			}

			//if unmap
			if (unmap) {
				//protect
				OS::mprotect(this->baseAddress + offset, size, false, false);

				//unamp
				OS::madviseDontNeed(this->baseAddress + offset, size);

				//mark unmapped
				status.mapped = false;
			}
		}

		//unlock
		for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
			if (toLock[i])
				this->segmentMutexes[i].unlock();
	}
}

/*******************  FUNCTION  *********************/
void Mapping::prefetch(size_t offset, size_t size)
{

}

/*******************  FUNCTION  *********************/
void Mapping::evict(Policy * sourcePolicy, size_t segmentId)
{
	//notify policies
	if (sourcePolicy != localPolicy && localPolicy != NULL)
		localPolicy->notifyEvict(this, segmentId);
	if (sourcePolicy != globalPolicy && globalPolicy != NULL)
		globalPolicy->notifyEvict(this, segmentId);
	
	//flush memory
	flush(segmentId * segmentSize, segmentSize, true);
}

/*******************  FUNCTION  *********************/
void Mapping::skipFirstRead(void)
{
	for (size_t i = 0 ; i < this->segmentSize ; i++)
		this->segmentStatus[i].needRead = false;
}

/*******************  FUNCTION  *********************/
size_t Mapping::getSegmentSize(void) const
{
	return this->segmentSize;
}