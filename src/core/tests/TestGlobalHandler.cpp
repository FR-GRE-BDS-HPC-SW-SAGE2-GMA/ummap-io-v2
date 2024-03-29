/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
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
void failure_handler(int sig, siginfo_t *si, void *context)
{
	//extract
	fprintf(stderr, "Expected failure\n");
	exit(1);
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, handler_stacking)
{
	GlobalHandler * handler = new GlobalHandler;
	setGlobalHandler(handler);

	//set preexisting handler
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = failure_handler;
	sigfillset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, NULL);

	//generate fa
	ASSERT_DEATH(*(char*)0x2 = 'a', "Expected failure");

	//set ummap handler
	setupSegfaultHandler();
	ASSERT_DEATH(*(char*)0x2 = 'a', "Expected failure");

	//set ummap handler
	unsetSegfaultHandler();
	ASSERT_DEATH(*(char*)0x2 = 'a', "Expected failure");

	//clear
	signal(SIGSEGV, SIG_DFL);
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
	Mapping mapping(NULL, 8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ, UMMAP_DEFAULT, driver);
	getGlobalhandler()->registerMapping(&mapping);

	//read access
	char * ptr = (char*)mapping.getAddress();
	for (int i = 0 ; i < 8*UMMAP_PAGE_SIZE ; i++)
		ASSERT_EQ(32, ptr[i]);

	//clean
	getGlobalhandler()->unregisterMapping(&mapping);
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
	Mapping mapping(NULL, 8 * UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, driver);
	getGlobalhandler()->registerMapping(&mapping);

	//read access
	char * ptr = (char*)mapping.getAddress();
	for (int i = 0 ; i < 8 * UMMAP_PAGE_SIZE ; i++)
		ptr[i] = 48;

	//read access
	for (int i = 0 ; i < 8 * UMMAP_PAGE_SIZE ; i++)
		ASSERT_EQ(48, ptr[i]);

	//clean
	getGlobalhandler()->unregisterMapping(&mapping);
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
	Mapping mapping(NULL, 8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ | PROT_WRITE, UMMAP_DEFAULT, driver);
	getGlobalhandler()->registerMapping(&mapping);

	//read access
	char * ptr = (char*)mapping.getAddress();
	bool ok = true;
	#pragma omp parallel shared(ok)
	{
		#pragma omp barrier
		for (size_t i = 0 ; i < 8*UMMAP_PAGE_SIZE ; i++) {
			if (ptr[i] != 32) {
				ok = false;
			}
		}
	}
	ASSERT_TRUE(ok);

	//clean
	getGlobalhandler()->unregisterMapping(&mapping);
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
	Mapping mapping(NULL, 8 * UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ | PROT_WRITE, UMMAP_DEFAULT, driver);
	getGlobalhandler()->registerMapping(&mapping);

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
	getGlobalhandler()->unregisterMapping(&mapping);
	unsetSegfaultHandler();
	clearGlobalHandler();
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, policy)
{
	FifoPolicy * policy = new FifoPolicy(4096, false);
	GlobalHandler handler;

	handler.registerPolicy("test", policy);
	ASSERT_EQ(policy, handler.getPolicy("test", true));
	ASSERT_EQ(NULL, handler.getPolicy("test2", true));

	handler.unregisterPolicy("test");
	ASSERT_EQ(NULL, handler.getPolicy("test", true));
}


/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, mmap_and_unmap)
{
	//setup
	GlobalHandler handler;

	//map 2
	void * ptr1 = handler.ummap(NULL, 8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, 0, new DummyDriver(16), NULL, "none");
	handler.ummap(NULL, 8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, 0, new DummyDriver(16), NULL, "none");

	//destroy 1
	ASSERT_EQ(0, handler.uunmap(ptr1,0 ));
}

/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, mmap_and_policy)
{
	//setup
	GlobalHandler handler;

	//create policy
	handler.registerPolicy("global", new FifoPolicy(4*UMMAP_PAGE_SIZE, false));

	//map 2
	void * ptr1 = handler.ummap(NULL, 8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, 0, new DummyDriver(16), NULL, "global");
	handler.ummap(NULL, 8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, 0, new DummyDriver(16), NULL, "global");

	//destroy 1
	ASSERT_EQ(0, handler.uunmap(ptr1, 0));
}


/*******************  FUNCTION  *********************/
TEST(TestGlobalHandler, deleteAllMappings)
{
	//setup global
	GlobalHandler handler;

	//mapping
	DummyDriver * driver = new DummyDriver(32);
	Mapping * mapping = new Mapping(NULL, 8 * UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, driver);
	handler.registerMapping(mapping);

	//call
	handler.deleteAllMappings();
}
