/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//gtest
#include <gtest/gtest.h>
//local
#include "../../portability/OS.hpp"
#include "../../drivers/DummyDriver.hpp"
#include "../../policies/FifoPolicy.hpp"
#include "../GlobalHandler.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, constructor)
{
	GlobalHandler handler;
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, setup)
{
	GlobalHandler * handler = new GlobalHandler;
	setGlobalHandler(handler);
	setupSegfaultHandler();
	unsetSegfaultHandler();
	clearGlobalHandler();
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, basic_read_workflow)
{
	//setup global
	GlobalHandler * handler = new GlobalHandler();
	setGlobalHandler(handler);
	setupSegfaultHandler();

	//mapping
	DummyDriver * driver = new DummyDriver(32);
	Mapping mapping(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, MAPPING_PROT_READ, driver);
	gblHandler->registerMapping(&mapping);

	//read access
	char * ptr = (char*)mapping.getAddress();
	for (int i = 0 ; i < 8*UMMAP_PAGE_SIZE ; i++)
		ASSERT_EQ(32, ptr[i]);

	//clean
	gblHandler->unregisterMapping(&mapping);
	unsetSegfaultHandler();
	clearGlobalHandler();
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, basic_write_workflow)
{
	//setup global
	GlobalHandler * handler = new GlobalHandler();
	setGlobalHandler(handler);
	setupSegfaultHandler();

	//mapping
	DummyDriver * driver = new DummyDriver(32);
	Mapping mapping(8 * UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, MAPPING_PROT_RW, driver);
	gblHandler->registerMapping(&mapping);

	//read access
	char * ptr = (char*)mapping.getAddress();
	for (int i = 0 ; i < 8 * UMMAP_PAGE_SIZE ; i++)
		ptr[i] = 48;
	
	//read access
	for (int i = 0 ; i < 8 * UMMAP_PAGE_SIZE ; i++)
		ASSERT_EQ(48, ptr[i]);

	//clean
	gblHandler->unregisterMapping(&mapping);
	unsetSegfaultHandler();
	clearGlobalHandler();
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, basic_read_workflow_parallel)
{
	//setup global
	GlobalHandler * handler = new GlobalHandler();
	setGlobalHandler(handler);
	setupSegfaultHandler();

	//mapping
	DummyDriver * driver = new DummyDriver(32);
	Mapping mapping(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, MAPPING_PROT_READ, driver);
	gblHandler->registerMapping(&mapping);

	//read access
	char * ptr = (char*)mapping.getAddress();
	bool ok = true;
	#pragma omp parallel shared(ok)
	{
		#pragma omp barrier
		for (int i = 0 ; i < 8*UMMAP_PAGE_SIZE ; i++) {
			if (ptr[i] != 32)
				ok = false;
		}
	}
	ASSERT_TRUE(ok);

	//clean
	gblHandler->unregisterMapping(&mapping);
	unsetSegfaultHandler();
	clearGlobalHandler();
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, basic_write_workflow_parallel)
{
	//setup global
	GlobalHandler * handler = new GlobalHandler;
	setGlobalHandler(handler);
	setupSegfaultHandler();

	//mapping
	DummyDriver * driver = new DummyDriver(32);
	Mapping mapping(8 * UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, MAPPING_PROT_RW, driver);
	gblHandler->registerMapping(&mapping);

	//read access
	char * ptr = (char*)mapping.getAddress();
	#pragma omp parallel
	{
		#pragma omp barrier
		for (int i = 0 ; i < 8 * UMMAP_PAGE_SIZE ; i++)
			ptr[i] = 48;
	}
	
	//read access
	for (int i = 0 ; i < 8 * UMMAP_PAGE_SIZE ; i++)
		ASSERT_EQ(48, ptr[i]);

	//clean
	gblHandler->unregisterMapping(&mapping);
	unsetSegfaultHandler();
	clearGlobalHandler();
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, policy)
{
	FifoPolicy * policy = new FifoPolicy(4096, false);
	GlobalHandler handler;
	
	handler.registerPolicy("test", policy);
	ASSERT_EQ(policy, handler.getPolicy("test"));
	ASSERT_EQ(NULL, handler.getPolicy("test2"));

	handler.unregisterPolicy("test");
	ASSERT_EQ(NULL, handler.getPolicy("test"));
}


/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, mmap_and_unmap)
{
	//setup
	GlobalHandler handler;

	//map 2
	void * ptr1 = handler.ummap(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, MAPPING_PROT_RW, new DummyDriver(16), NULL, "none");
	void * ptr2 = handler.ummap(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, MAPPING_PROT_RW, new DummyDriver(16), NULL, "none");

	//destroy 1
	ASSERT_EQ(0, handler.umunmap(ptr1));
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, mmap_and_policy)
{
	//setup
	GlobalHandler handler;

	//create policy
	handler.registerPolicy("global", new FifoPolicy(4*UMMAP_PAGE_SIZE, false));

	//map 2
	void * ptr1 = handler.ummap(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, MAPPING_PROT_RW, new DummyDriver(16), NULL, "global");
	void * ptr2 = handler.ummap(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, MAPPING_PROT_RW, new DummyDriver(16), NULL, "global");

	//destroy 1
	ASSERT_EQ(0, handler.umunmap(ptr1));
}

