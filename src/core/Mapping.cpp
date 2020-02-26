/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstring>
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
	this->driver = driver;
	this->localPolicy = localPolicy;
	this->globalPolicy = globalPolicy;
	this->protection = protection;

	//establish mapping
	this->baseAddress = OS::mmapProtNone(size);
	this->segments = size / segmentSize;

	//establish state tracking
	this->status = new SegmentStatus[this->segments];
	memset(this->status, 0, sizeof(SegmentStatus) * this->segments);
	for (size_t i = 0 ; i < this->segmentSize ; i++)
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
	this->segmentMutexes = new std::mutex[this->segmentMutexesCnt];
}

/*******************  FUNCTION  *********************/
Mapping::~Mapping(void)
{
	
}

/*******************  FUNCTION  *********************/
void * Mapping::getAddress(void)
{
	return this->baseAddress;
}

/*******************  FUNCTION  *********************/
void Mapping::onSegmentationFault(void * address, bool isWrite)
{
	//checks
	assume(address >= this->baseAddress 
		&& address < (char*)this->baseAddress + this->segments * this->segmentSize,
		"Invalid address, not fit into the current segment !");

	//cal segment id
	size_t segmentId = ((char*)address - (char*)this->baseAddress) / this->segmentSize;
	
	//CRITICAL SECTION
	{
		//lock to access
		int mutexId = segmentId % this->segmentMutexesCnt;
		std::lock_guard<std::mutex> lockGuard(this->segmentMutexes[mutexId]);

		//check status
		SegmentStatus & status = this->status[segmentId];

		//need to read
		if (status.needRead) {
			//TODO
		}
	}
}

/*******************  FUNCTION  *********************/
void Mapping::flush(void)
{

}

/*******************  FUNCTION  *********************/
void Mapping::flush(size_t offset, size_t size)
{

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
