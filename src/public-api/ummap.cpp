/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <cassert>
#include "../core/GlobalHandler.hpp"
#include "../drivers/DummyDriver.hpp"
#include "ummap.h"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
void ummap_init(void)
{
	ummapio::GlobalHandler * handler = new ummapio::GlobalHandler();
	ummapio::setGlobalHandler(handler);
	ummapio::setupSegfaultHandler();
}

/*******************  FUNCTION  *********************/
void ummap_finalize(void)
{
	ummapio::unsetSegfaultHandler();
	ummapio::clearGlobalHandler();
}

/*******************  FUNCTION  *********************/
void * ummap(size_t size, size_t segment_size, size_t storage_offset, ummap_mapping_prot_t protection, ummap_driver_t * driver, ummap_policy_t * local_policy, const char * policy_group)
{
	//check
	assert(gblHandler != NULL);

	//convert
	MappingProtection prot = (MappingProtection)protection;
	Driver * driv = (Driver*)(driver);
	Policy * pol = (Policy*)(local_policy);

	//call & ret
	return gblHandler->ummap(size, segment_size, storage_offset, prot, driv, pol, policy_group);
}

/*******************  FUNCTION  *********************/
int umunmap(void * ptr)
{
	//check
	assert(gblHandler != NULL);
	assert(ptr != NULL);

	//call
	return gblHandler->umunmap(ptr);
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_dummy(char value)
{
	Driver * driver = new DummyDriver(value);
	return (ummap_driver_t*)driver;
}
