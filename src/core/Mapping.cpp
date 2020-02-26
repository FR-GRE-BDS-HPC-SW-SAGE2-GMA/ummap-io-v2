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
