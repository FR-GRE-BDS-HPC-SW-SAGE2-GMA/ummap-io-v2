/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include "Mapping.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
Mapping::Mapping(size_t size, size_t segmentSize, Driver * driver, Policy * localPolicy = NULL, Policy * globalPolicy = NULL)
{

}

/*******************  FUNCTION  *********************/
Mapping::~Mapping(void)
{
	
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
