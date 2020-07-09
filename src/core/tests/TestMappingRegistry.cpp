/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//gtest
#include <gtest/gtest.h>
//local
#include "drivers/DummyDriver.hpp"
#include "portability/OS.hpp"
#include "../MappingRegistry.hpp"

/********************  HEADERS  *********************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestRegistry, constructor)
{
	MappingRegistry registry;
}

/*******************  FUNCTION  *********************/
TEST(TestRegistry, reg)
{
	//deps
	DummyDriver * driver = new DummyDriver;
	Mapping mapping(10*1024*1024, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, driver);

	//create
	MappingRegistry registry;
	
	//fill
	registry.registerMapping(&mapping);

	//check
	char * base = (char*)mapping.getAddress();
	ASSERT_EQ(&mapping, registry.getMapping(base));
	ASSERT_EQ(&mapping, registry.getMapping(base+UMMAP_PAGE_SIZE));
	ASSERT_EQ(&mapping, registry.getMapping(base+10*1024*1024-1));

	//check not found
	char buffer[32];
	ASSERT_EQ(NULL, registry.getMapping(base+10*1024*1024));
	ASSERT_EQ(NULL, registry.getMapping(buffer));

	//unreg
	registry.unregisterMapping(&mapping);
	ASSERT_EQ(NULL, registry.getMapping(base));
}
