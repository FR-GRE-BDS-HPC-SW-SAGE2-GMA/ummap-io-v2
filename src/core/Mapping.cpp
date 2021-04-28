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
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Establish a new memory mapping.
 * @param addr Address hint where to establish the mapping like in standard mmap semantic.
 * @param size Define the size of the mapping. If can be a non multiple of segment size. In this case, the mapping itself
 * will be sized to the next multiple. For read/write operations it will ignore this extra sub-segment.
 * @param segmentSize Equivalent of the page size for a standard mmap, it define the granularity of the IO operations.
 * This size must be a multiple of the OS page size (4K).
 * @param storageOffset Offset to apply on the storage of reach the data to be mapped. It does not have to be aligned on
 * page size.
 * @param protection Define the access protection to assign to this mapping. It uses the flags from mmap so you can
 * use the given flags and 'or' them: PROT_READ, PROT_WRIT, PROT_EXEC.
 * @param flags Flags to enable of disable some behaviors of ummap-io. Currently valid flags are : UMMAP_NO_FIRST_READ, 
 * UMMAP_THREAD_UNSAFE. Go in their respective documentation to get more information on them. You can also use UMMAP_FIXED
 * to force the targetted address to establish the mapping.
 * @param driver Pointer to the given driver. It will be destroyed automatically depending on its status about auto clean.
 * @param localPolicy Define the local policy to be used.
 * @param globalPolicy Define the global policy to be used and shared between multiple mappings.
**/
Mapping::Mapping(void *addr, size_t size, size_t segmentSize, size_t storageOffset, int protection, int flags, Driver * driver, Policy * localPolicy, Policy * globalPolicy)
{
	//checks
	assume(size > 0, "Do not accept null size mapping");
	assume(segmentSize > 0, "Do not accept null segment size");
	assumeArg(segmentSize % UMMAP_PAGE_SIZE == 0, "Segment size should be multiple of page size (%1), got '%2'").arg(UMMAP_PAGE_SIZE).arg(size).end();

	//calc map size
	size_t mapSize = size;
	if (size % segmentSize != 0)
		mapSize += segmentSize - (size % segmentSize);
	assumeArg(mapSize % UMMAP_PAGE_SIZE == 0, "Size should be multiple of page size (%1), got '%2'").arg(UMMAP_PAGE_SIZE).arg(size).end();
	assumeArg(mapSize % segmentSize == 0, "Size should be multiple of segment size (%1), got '%2'").arg(segmentSize).arg(size).end();

	//set
	this->driver = driver;
	this->localPolicy = localPolicy;
	this->globalPolicy = globalPolicy;
	this->protection = protection;
	this->size = size;
	this->storageOffset = storageOffset;
	this->threadSafe = true;

	//pre check
	this->registerRange();

	//warning
	if (localPolicy != NULL && globalPolicy != NULL)
		localPolicy->forceUsingGroupMutex(globalPolicy->getLocalMutex());

	//no thread safe
	if (flags & UMMAP_THREAD_UNSAFE)
		this->threadSafe = false;
	
	//check thread safety
	if (this->threadSafe)
		assume(driver->checkThreadSafety(), "Ask for mapping thread safety but the driver does not support it !");

	//if use map fixed
	bool mapFixed = (flags & UMMAP_FIXED);

	//establish mapping
	this->baseAddress = (char*)driver->directMmap(addr, size, storageOffset, protection & PROT_READ, protection & PROT_WRITE, protection & PROT_EXEC, mapFixed);
	if (this->baseAddress == NULL)
		this->baseAddress = (char*)OS::mmapProtNone(addr, mapSize, mapFixed);

	//sizes
	this->segments = mapSize / segmentSize;
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
	if (static_cast<size_t>(this->segmentMutexesCnt) > this->segments)
		this->segmentMutexesCnt = this->segments;
	this->segmentMutexes = new std::mutex[this->segmentMutexesCnt];

	//no first read
	if (flags & UMMAP_NO_FIRST_READ)
		this->skipFirstRead();
}

/*******************  FUNCTION  *********************/
/**
 * Destructor of a mapping.
 * It unmap the mapping and destroy the local ressources.
**/
Mapping::~Mapping(void)
{
	//unregister mapping
	this->unregisterRange();

	//unmap
	if (driver->directMunmap(this->baseAddress, size, storageOffset) == false)
		OS::munmap(this->baseAddress, this->getAlignedSize());

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

	//destroy driver dup()
	if (this->driver->hasAutoclean())
		delete this->driver;
}

/*******************  FUNCTION  *********************/
void Mapping::directMmapCow(Driver * newDriver)
{
	//allocate a new one
	char * newAddr = (char*)newDriver->directMmap(this->baseAddress, this->size, this->storageOffset, this->protection & PROT_READ, this->protection & PROT_WRITE, this->protection & PROT_EXEC, false);
	assume(newAddr != NULL, "Try to call directMmapCow() on a driver not supporting direct mmap !");

	//copy
	memcpy(newAddr, baseAddress, this->size);

	//msync old before erase
	msync(baseAddress, size, MS_SYNC);

	//mremap to erase
	OS::mremapForced(newAddr, size, baseAddress);
}

/*******************  FUNCTION  *********************/
/**
 * Unregister the mapping range on the driver.
**/
void Mapping::unregisterRange(void)
{
	driver->erase_mapping(this->mappingDriverId, this->storageOffset, this->size, this->protection & PROT_WRITE);
	this->mappingDriverId = -1;
}

/*******************  FUNCTION  *********************/
/**
 * Register the mapping range on the driver.
**/
void Mapping::registerRange(void)
{
	this->mappingDriverId = driver->establish_mapping(storageOffset, size, protection & PROT_WRITE);	
	assume(this->mappingDriverId >= 0, "Faild to register range of the given mapping !");
}

/*******************  FUNCTION  *********************/
/**
 * Return the base address of the mapping.
**/
void * Mapping::getAddress(void)
{
	return this->baseAddress;
}

/*******************  FUNCTION  *********************/
/**
 * Disable the thread safety concerning the mprotect and mremap semantic.
 * It still keep the mutexes which are untouched by this action.
**/
void Mapping::disableThreadSafety(void)
{
	this->threadSafe = false;
}

/*******************  FUNCTION  *********************/
/**
 * On a first touch we need to allow access to the segment and load
 * the data. In order to get thread safety the data is first load in 
 * another segment which is then mremapped to override the current one.
 * This behavior is disabled by disableThreadSafety() and replaced
 * by a pure mprotect operation.
 * In some extreme pressure cases the mremap approach seems to increase
 * the memory used by the system.
**/
void Mapping::loadAndSwapSegment(size_t offset, bool writeAccess)
{
	//Rational: In order to be atomic we load the data in a segment, then
	//we mremap this segment on the expected one so it replace atomicaly
	//the old one and open access. This is to avoid multi-threading issue
	//if a second thread made first touch while the first one is reading the
	//data if we just made madvise on the pre-existing PROT_NONE segment.

	//map a page in RW access
	void * ptr = NULL;
	if (threadSafe) {
		ptr = OS::mmapProtFull(this->segmentSize, protection & PROT_EXEC);
	} else {
		ptr = (char*)this->baseAddress + offset;
		OS::mprotect(ptr, segmentSize, true, true, protection & PROT_EXEC);
	}

	//read inside new segment
	ssize_t res = this->driver->pread(ptr, readWriteSize(offset), this->storageOffset + offset);
	assumeArg(res >= 0, "Fail to read all data, got %1 instead of %2 !")
		.arg(res)
		.arg(segmentSize)
		.end();

	//make read only
	if (!writeAccess)
		OS::mprotect(ptr, segmentSize, true, false, protection & PROT_EXEC);
	
	//now remap to move the segment and override the PROT_NONE one
	if (threadSafe)
		OS::mremapForced(ptr, segmentSize, this->baseAddress + offset);
}

/*******************  FUNCTION  *********************/
/**
 * Recive notification of segmentation fault on a an address of the mapping.
 * In this case, we need to load the data from the storage and change the
 * access write. On write operation we need to mark the segment as dirty for
 * later flush.
 * @param address The faulty address to determine the touched segment.
 * @param isWrite Define if it is a write or read access.
**/
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
	SegmentStatus oldStatus;

	//check
	if (this->protection == PROT_NONE)
		UMMAP_FATAL("Try to access a segment which is PROT_NONE");
	if (isWrite && (this->protection & PROT_WRITE) == 0)
		UMMAP_FATAL("Try to write access a segment which is not writable");
	
	//CRITICAL SECTION
	{
		//lock to access
		int mutexId = segmentId % this->segmentMutexesCnt;
		std::lock_guard<std::mutex> lockGuard(this->segmentMutexes[mutexId]);

		//check status
		SegmentStatus & status = this->segmentStatus[segmentId];
		oldStatus = status;

		//already done
		if (isWrite == status.dirty && status.mapped)
			return;

		//if not mapped
		if (!status.mapped && status.needRead){
			//Load in a temp buffer and swap for atomicity
			this->loadAndSwapSegment(offset, isWrite);
			if (isWrite) {
				status.dirty = true;
			}
		} else if (isWrite) {
			//this is a write, open write access
			OS::mprotect(segmentBase, segmentSize, true, true, protection & PROT_EXEC);

			//mark dirty
			status.dirty = true;

			//update dirty time for latter flush operation
		} else {
			//this is a first touch withou need read, open as readonly
			OS::mprotect(segmentBase, segmentSize, true, false, protection & PROT_EXEC);
		}

		//mark as mapped
		status.mapped = true;
	}

	//notify eviction policy
	if (this->localPolicy != NULL)
		this->localPolicy->notifyTouch(this, segmentId, isWrite, oldStatus.mapped, oldStatus.dirty);
	if (this->globalPolicy != NULL)
		this->globalPolicy->notifyTouch(this, segmentId, isWrite, oldStatus.mapped, oldStatus.dirty);
}

/*******************  FUNCTION  *********************/
/**
 * Get the status of the given segment identified by its offset.
 * @param offset Offset of the segment for which we want the status.
 * @return Retuen the status of the given segment.
**/
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
/**
 * Return the size of the mapping (not aligned to segment size).
**/
size_t Mapping::getSize(void) const
{
	return this->size;
}

/*******************  FUNCTION  *********************/
/**
 * Retun the size of the memory mapping (aligned on segment size)
**/
size_t Mapping::getAlignedSize(void) const
{
	return this->segmentSize * this->segments;
}

/*******************  FUNCTION  *********************/
/**
 * Return the list of mutex to be taken to cover the given memory range.
 * @param offset to start with.
 * @param size Range to consider.
 * @param buffer A potential static buffer to be used to avoid allocation.
 * @param bufferSize Size of the static buffer to check if enough.
**/
const bool * Mapping::getMutexRange(size_t offset, size_t size, bool * buffer, size_t bufferSize) const
{
	//check
	assert(offset < this->getSize());
	assert(offset % this->segmentSize == 0);
	assert(offset + size <= this->getAlignedSize());
	assert(size % this->segmentSize == 0);

	//allocate
	bool * list = buffer;
	if (list == NULL || bufferSize < this->segmentMutexesCnt)
		list = new bool[this->segmentMutexesCnt];

	//init
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
/**
 * Apply a sync operation on the wall segment.
**/
void Mapping::flush(bool sync)
{
	int flags = UMMAP_FLUSH_DEFAULT;
	if (sync)
		flags |= UMMAP_FLUSH_SYNC;
	this->flush(0, getSize(), flags);
}

/*******************  FUNCTION  *********************/
/**
 * Used to handle the read/write operations to handle the end of the
 * mapping which is no a multiple of the segment size.
 * @param offset Offset from where to read/write.
 * @return Return the size of a segment if not at then end of the mapping
 * and return the required size otherwise.
**/
size_t Mapping::readWriteSize(size_t offset)
{
	if (size - offset >= segmentSize)
		return segmentSize;
	else
		return size - offset;
}

/*******************  FUNCTION  *********************/
/**
 * Drop the clean pages from the memory.
**/
void Mapping::dropClean(void)
{
	//CRITICAL SECTION
	{
		//lock the whole segment
		for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
			this->segmentMutexes[i].lock();

		//loop on all
		for (size_t i = 0 ; i < this->segments ; i++) {
			//get segment
			SegmentStatus & status = this->segmentStatus[i];

			//check if drop
			if (status.mapped && status.dirty == false) {
				//calc addr
				void * addr = this->baseAddress + this->segmentSize * i;

				//protect
				OS::mprotect(addr, segmentSize, false, false, protection & PROT_EXEC);

				//unamp
				OS::madviseDontNeed(addr, segmentSize);

				//mark unmapped
				status.mapped = false;
			}
		}

		//unlock the whole segment
		for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
			this->segmentMutexes[i].unlock();
	}
}

/*******************  FUNCTION  *********************/
/**
 * Drop the clean pages from the memory.
**/
void Mapping::markCleanAsDirty(void)
{
	//CRITICAL SECTION
	{
		//lock the whole segment
		for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
			this->segmentMutexes[i].lock();

		//loop on all
		for (size_t i = 0 ; i < this->segments ; i++) {
			//get segment
			SegmentStatus & status = this->segmentStatus[i];

			//check if drop
			if (status.mapped && status.dirty == false && (protection & PROT_WRITE)) {
				//calc addr
				void * addr = this->baseAddress + this->segmentSize * i;

				//protect
				OS::mprotect(addr, segmentSize, true, true, protection & PROT_EXEC);

				//mark unmapped
				status.dirty = true;
			}
		}

		//unlock the whole segment
		for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
			this->segmentMutexes[i].unlock();
	}
}

/*******************  FUNCTION  *********************/
/**
 * Apply a sync operation.
 * @param offset Sync from the given offset.
 * @param size Define the range of the memory to sync.
 * @param flags Define flags. You can look on :
 *  - UMMAP_FLUSH_DEFAULT
 *  - UMMAP_FLUSH_SYNC
 *  - UMMAP_FLUSH_UNMAP
 *  - UMMAP_FLUSH_NO_LOCK
**/
void Mapping::flush(size_t offset, size_t size, int flags)
{
	//check
	assumeArg(offset < this->getSize(), "Offset (%1) is not in valid range !").arg(offset).end();
	assumeArg(offset + size <= this->getAlignedSize(), "'Offset (%1) + size' is not in valid range !").arg(offset).end();
	assumeArg(offset % segmentSize == 0, "Should get offset (%1) multiple of segment size !").arg(offset).end();
	assumeArg(size % segmentSize == 0, "Should get size (%1) multiple of segment size !").arg(size).end();

	//default flags
	bool sync = false;
	bool unmap = false;
	bool lock = true;

	//apply flags
	if (flags & UMMAP_FLUSH_SYNC) sync = true;
	if (flags & UMMAP_FLUSH_UNMAP) unmap = true;
	if (flags & UMMAP_FLUSH_NO_LOCK) lock = false;

	//direct sync
	bool res = driver->directMSync((char*)getAddress() + offset, size, storageOffset);
	if (res)
		return;

	//what to lock
	const int stackToLockSize = 2048;
	bool stackToLock[stackToLockSize];
	const bool * toLock = getMutexRange(offset, size, stackToLock, stackToLockSize);

	//CRITICAL SECTION
	{
		//lock the whole segment
		//@TODO Can search to lock only related pages
		if (lock)
			for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
				if (toLock[i])
					this->segmentMutexes[i].lock();

		//loop on all
		//@TODO: bulk operation
		for (size_t curOffset = offset ; curOffset < offset + size ; curOffset += this->segmentSize) {
			size_t id = curOffset / this->segmentSize;
			//check status
			SegmentStatus & status = this->segmentStatus[id];
			if (status.dirty && status.mapped) {
				//compute
				void * segmentPtr = this->baseAddress + curOffset;

				//mprotect the whole considered segment
				//BUG: on centos/redhat7, this mprotect leads to a kernel live lock
				//     when used with IOC driver. Cannot IB register a segment which
				//     is read only. It make the process un-killable.
				//     The problem seems fixed in centos/redhat 8.
				if (threadSafe)
					OS::mprotect(this->baseAddress + curOffset, segmentSize, true, !threadSafe/*TODO*/, protection & PROT_EXEC);

				//apply
				ssize_t res = this->driver->pwrite(segmentPtr, readWriteSize(curOffset), this->storageOffset + curOffset);

				//errors
				assumeArg(res != -1, "Fail to pwrite : %1").argStrErrno().end();
				assumeArg(res >= 0, "Fail to fully write the segment, got : %1").arg(res).end();

				//update status
				status.dirty = false;
				status.needRead = true;
			}

			if (status.mapped) {
				//if unmap
				if (unmap) {
					//protect
					OS::mprotect(this->baseAddress + curOffset, segmentSize, false, false, protection & PROT_EXEC);

					//unamp
					OS::madviseDontNeed(this->baseAddress + curOffset, segmentSize);

					//mark unmapped
					status.mapped = false;
				} else if (!threadSafe) {
					OS::mprotect(this->baseAddress + curOffset, segmentSize, true, false/*TODO*/, protection & PROT_EXEC);
				}
			}
		}

		//sync
		if (sync)
			driver->sync(getAddress(), offset, size);

		//unlock
		if (lock)
			for (int i = 0 ; i < this->segmentMutexesCnt ; i++)
				if (toLock[i])
					this->segmentMutexes[i].unlock();
		
		if (toLock != stackToLock)
			delete toLock;
	}
}

/*******************  FUNCTION  *********************/
/**
 * Use to prefeatch the given range. But not implemented yet.
**/
void Mapping::prefetch(size_t offset, size_t size)
{

}

/*******************  FUNCTION  *********************/
/**
 * When evicting a segment we need to notify the local
 * and global policies.
 * @param sourcePolicy Define the policy which has initiated the eviction
 * operation of NULL if none. This is used to not self notify itself.
 * @param segmentId Define the ID of the segment to be evicted.
**/
void Mapping::evict(Policy * sourcePolicy, size_t segmentId)
{
	//CRITICAL SECTION
	{
		//lock to access
		int mutexId = segmentId % this->segmentMutexesCnt;
		std::lock_guard<std::mutex> lockGuard(this->segmentMutexes[mutexId]);


		//notify policies
		if (sourcePolicy != localPolicy && localPolicy != NULL)
			localPolicy->notifyEvict(this, segmentId);
		if (sourcePolicy != globalPolicy && globalPolicy != NULL)
			globalPolicy->notifyEvict(this, segmentId);
	
		//flush memory
		flush(segmentId * segmentSize, segmentSize, UMMAP_FLUSH_UNMAP | UMMAP_FLUSH_NO_LOCK);
	}
}

/*******************  FUNCTION  *********************/
/**
 * Set the skip first read flag on all segments. In this case the memory
 * will contain zeroes on the first access and not the data from storage.
**/
void Mapping::skipFirstRead(void)
{
	for (size_t i = 0 ; i < this->segments ; i++)
		this->segmentStatus[i].needRead = false;
}

/*******************  FUNCTION  *********************/
/**
 * Return the segment size in use.
**/
size_t Mapping::getSegmentSize(void) const
{
	return this->segmentSize;
}

/*******************  FUNCTION  *********************/
/**
 * Return the address of the current driver in use.
**/
Driver * Mapping::getDriver(void)
{
	return this->driver;
}

/*******************  FUNCTION  *********************/
void Mapping::copyExtraNotMappedPart(char * buffer, Driver * newDriver, size_t offset, size_t size)
{
	//cars
	ssize_t status;

	//check
	assert(newDriver != NULL);
	assert(buffer != NULL);

	//copy part before mapping
	for (size_t i = 0 ; i < size ; i+= this->segmentSize) {
		//compute copy size
		size_t copySize = this->segmentSize;
		if (i + this->segmentSize > size)
			copySize = size - i;
		//copy
		status = this->driver->pread(buffer, copySize, offset+i);
		assume(status == copySize, "Failed to read data from the original driver !");
		status = newDriver->pwrite(buffer, copySize, offset+i);
		assume(status == copySize, "Failed to write data from the target driver !");
	}
}

/*******************  FUNCTION  *********************/
void Mapping::copyMappedPart(char * buffer, Driver * newDriver, size_t storageSize)
{
	//vars
	ssize_t status;

	//check
	assert(newDriver != NULL);
	assert(buffer != NULL);

	//real copy size if end of mapping was never flushed to the origin file
	size_t sizeToDump = this->size;
	if (sizeToDump > storageSize)
		sizeToDump = storageSize;

	//copy part in mapping.
	for (size_t i = 0 ; i < sizeToDump ; i += this->segmentSize) {
		//compute copy size
		size_t copySize = this->segmentSize;
		if (i + this->segmentSize > sizeToDump)
			copySize = sizeToDump - i;
		
		//get segment info
		SegmentStatus & curStatus = this->segmentStatus[i/this->segmentSize];
		
		//apply copy mode if already have synced data in memory
		const size_t offset = this->storageOffset + i;
		if (curStatus.mapped == true && curStatus.dirty == false) {
			char * addr = this->baseAddress + offset;
			status = newDriver->pwrite(addr, copySize, offset);
			assume(status == copySize, "Failed to write data from the target driver !");
		} else {
			status = this->driver->pread(buffer, copySize, offset);
			assume(status == copySize, "Failed to read data from the original driver !");
			status = newDriver->pwrite(buffer, copySize, offset);
			assume(status == copySize, "Failed to write data from the target driver !");
		}
	}
}

/*******************  FUNCTION  *********************/
/**
 * Return the current offset in the object storage.
**/
size_t Mapping::getStorageOffset(void) const
{
	return this->storageOffset;
}

/*******************  FUNCTION  *********************/
void Mapping::copyToDriver(Driver * newDriver, size_t storageSize)
{
	//check
	assume(newDriver != NULL, "Got invalid NULL driver !");
	assume(storageSize >= this->storageOffset, "Try to copy and change driver with a size which is smaller than the mapping size !");

	//allocate buffer
	char * buffer = new char[this->segmentSize];

	//copy first part
	this->copyExtraNotMappedPart(buffer, newDriver, 0, this->storageOffset);

	//copy mapped part
	this->copyMappedPart(buffer, newDriver, storageSize - this->storageOffset);

	//copy extract part after the mapped one if needed
	const size_t endOffset = this->storageOffset+this->size;
	const size_t endSize = storageSize - endOffset;
	this->copyExtraNotMappedPart(buffer, newDriver, endOffset, endSize);
}

/*******************  FUNCTION  *********************/
#ifdef HAVE_HTOPML
/**
 * When htopml is enabled this function is used to dump the mapping state in a json format.
**/
void ummapio::convertToJson(htopml::JsonState & json,const SegmentStatus & value)
{
	json.openStruct();
		//json.printField("time", value.time);
		json.printField("mapped", value.mapped);
		json.printField("dirty", value.dirty);
		json.printField("needRead", value.needRead);
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
/**
 * When htopml is enabled this function is used to dump the mapping state in a json format.
**/
void ummapio::convertToJson(htopml::JsonState & json,const Mapping & value)
{
	json.openStruct();
		json.printField("size", value.size);
		json.printField("segments", value.segments);
		json.printField("segmentSize", value.segmentSize);
		json.printField("offset", value.storageOffset);
		json.printField("driverUri", value.driver->getUri());
		if (value.localPolicy == NULL)
			json.printField("localPolicyUri", "none://");
		else
			json.printField("localPolicyUri", value.localPolicy->getUri());
		if (value.globalPolicy == NULL)
			json.printField("globalPolicyUri", "none://");
		else
			json.printField("globalPolicyUri", value.globalPolicy->getUri());
		json.openFieldArray("status");
		for (size_t i = 0 ; i < value.segments ; i++)
			json.printValue(value.segmentStatus[i]);
		json.closeFieldArray("status");
	json.closeStruct();
}
#endif //HAVE_HTOPML
