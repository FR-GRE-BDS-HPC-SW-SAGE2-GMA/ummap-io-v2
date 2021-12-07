/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
//gtest
#include <gtest/gtest.h>
//local
#include "../Mapping.hpp"
#include "../Policy.hpp"
#include "../PolicyQuotaLocal.hpp"
#include "../../policies/FifoPolicy.hpp"
#include "../../policies/LifoPolicy.hpp"
#include "../../policies/FifoWindowPolicy.hpp"
#include "../../drivers/DummyDriver.hpp"
#include "../../portability/OS.hpp"

/********************  MACROS  **********************/
#define PAGE_SIZE 4096

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestPolicyQuotaLocal, register)
{
	PolicyQuotaLocal quota(4*UMMAP_PAGE_SIZE);
	FifoPolicy * policy = new FifoPolicy(4*UMMAP_PAGE_SIZE, true);
	size_t size = 8*UMMAP_PAGE_SIZE;

	//mapping
	DummyDriver driver(0);
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, policy, NULL);

	//register
	quota.registerPolicy(policy);
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyQuotaLocal, quota_not_reached)
{
	PolicyQuotaLocal quota(2*UMMAP_PAGE_SIZE);
	FifoPolicy * policy = new FifoPolicy(2*UMMAP_PAGE_SIZE, true);
	size_t size = 8*UMMAP_PAGE_SIZE;

	//mapping
	DummyDriver driver(0);
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, policy, NULL);

	//register
	quota.registerPolicy(policy);

	//loop
	char * ptr = (char*)mapping.getAddress();
	mapping.onSegmentationFault(ptr + 0 * UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr + 1 * UMMAP_PAGE_SIZE, true);
	EXPECT_EQ(2*UMMAP_PAGE_SIZE, policy->getCurrentMemory());

	//loop
	for (int i = 2 ; i < 8 ; i++) {
		mapping.onSegmentationFault(ptr + i * UMMAP_PAGE_SIZE, true);
		EXPECT_EQ(2*UMMAP_PAGE_SIZE, policy->getCurrentMemory()) << "Iteration " << i;
	}
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyQuotaLocal, quota_reach_one)
{
	PolicyQuotaLocal quota(2*UMMAP_PAGE_SIZE);
	FifoPolicy * policy = new FifoPolicy(4*UMMAP_PAGE_SIZE, true);
	size_t size = 8*UMMAP_PAGE_SIZE;

	//mapping
	DummyDriver driver(0);
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, policy, NULL);

	//register
	quota.registerPolicy(policy);

	//loop
	char * ptr = (char*)mapping.getAddress();
	mapping.onSegmentationFault(ptr + 0 * UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr + 1 * UMMAP_PAGE_SIZE, true);
	EXPECT_EQ(2*UMMAP_PAGE_SIZE, policy->getCurrentMemory());

	//loop
	for (int i = 2 ; i < 8 ; i++) {
		mapping.onSegmentationFault(ptr + i * UMMAP_PAGE_SIZE, true);
		EXPECT_EQ(2*UMMAP_PAGE_SIZE, policy->getCurrentMemory()) << "Iteration " << i;
	}
}


/*******************  FUNCTION  *********************/
TEST(TestPolicyQuotaLocal, quota_reach_two)
{
	size_t size = 8*UMMAP_PAGE_SIZE;
	PolicyQuotaLocal quota(2*UMMAP_PAGE_SIZE);

	//mapping
	DummyDriver driver1(0);
	FifoPolicy * policy1 = new FifoPolicy(4*UMMAP_PAGE_SIZE, true);
	Mapping mapping1(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver1, policy1, NULL);

	//mapping
	DummyDriver driver2(0);
	FifoPolicy * policy2 = new FifoPolicy(4*UMMAP_PAGE_SIZE, true);
	Mapping mapping2(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver2, policy2, NULL);

	//register
	quota.registerPolicy(policy1);
	quota.registerPolicy(policy2);

	//loop
	char * ptr1 = (char*)mapping1.getAddress();
	char * ptr2 = (char*)mapping2.getAddress();
	mapping1.onSegmentationFault(ptr1 + 0 * UMMAP_PAGE_SIZE, true);
	mapping2.onSegmentationFault(ptr2 + 0 * UMMAP_PAGE_SIZE, true);
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy1->getCurrentMemory());
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy2->getCurrentMemory());

	//loop
	for (int i = 1 ; i < 8 ; i++) {
		mapping1.onSegmentationFault(ptr1 + i * UMMAP_PAGE_SIZE, true);
		mapping2.onSegmentationFault(ptr2 + i * UMMAP_PAGE_SIZE, true);
		EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy1->getCurrentMemory()) << "Iteration " << i;
		EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy2->getCurrentMemory()) << "Iteration " << i;
	}
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyQuotaLocal, quota_reach_two_unbalanced)
{
	size_t size = 8*UMMAP_PAGE_SIZE;
	PolicyQuotaLocal quota(4*UMMAP_PAGE_SIZE);

	//mapping
	DummyDriver driver1(0);
	FifoPolicy * policy1 = new FifoPolicy(8*UMMAP_PAGE_SIZE, true);
	Mapping mapping1(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver1, policy1, NULL);

	//mapping
	DummyDriver driver2(0);
	FifoPolicy * policy2 = new FifoPolicy(8*UMMAP_PAGE_SIZE, true);
	Mapping mapping2(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver2, policy2, NULL);

	//register
	quota.registerPolicy(policy1);
	quota.registerPolicy(policy2);

	//touch first on each
	char * ptr1 = (char*)mapping1.getAddress();
	char * ptr2 = (char*)mapping2.getAddress();
	mapping1.onSegmentationFault(ptr1 + 0 * UMMAP_PAGE_SIZE, true);
	mapping2.onSegmentationFault(ptr2 + 0 * UMMAP_PAGE_SIZE, true);
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy1->getCurrentMemory());
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy2->getCurrentMemory());

	//touch many on mapping 2
	mapping2.onSegmentationFault(ptr2 + 1 * UMMAP_PAGE_SIZE, true);
	mapping2.onSegmentationFault(ptr2 + 2 * UMMAP_PAGE_SIZE, true);
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy1->getCurrentMemory());
	EXPECT_EQ(3*UMMAP_PAGE_SIZE, policy2->getCurrentMemory());
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyQuotaLocal, quota_reach_two_unbalanced_lifo)
{
	size_t size = 8*UMMAP_PAGE_SIZE;
	PolicyQuotaLocal quota(4*UMMAP_PAGE_SIZE);

	//mapping
	DummyDriver driver1(0);
	LifoPolicy * policy1 = new LifoPolicy(8*UMMAP_PAGE_SIZE, true);
	Mapping mapping1(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver1, policy1, NULL);

	//mapping
	DummyDriver driver2(0);
	LifoPolicy * policy2 = new LifoPolicy(8*UMMAP_PAGE_SIZE, true);
	Mapping mapping2(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver2, policy2, NULL);

	//register
	quota.registerPolicy(policy1);
	quota.registerPolicy(policy2);

	//touch first on each
	char * ptr1 = (char*)mapping1.getAddress();
	char * ptr2 = (char*)mapping2.getAddress();
	mapping1.onSegmentationFault(ptr1 + 0 * UMMAP_PAGE_SIZE, true);
	mapping2.onSegmentationFault(ptr2 + 0 * UMMAP_PAGE_SIZE, true);
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy1->getCurrentMemory());
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy2->getCurrentMemory());

	//touch many on mapping 2
	mapping2.onSegmentationFault(ptr2 + 1 * UMMAP_PAGE_SIZE, true);
	mapping2.onSegmentationFault(ptr2 + 2 * UMMAP_PAGE_SIZE, true);
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy1->getCurrentMemory());
	EXPECT_EQ(3*UMMAP_PAGE_SIZE, policy2->getCurrentMemory());
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyQuotaLocal, quota_reach_two_unbalanced_fifo_window)
{
	size_t size = 8*UMMAP_PAGE_SIZE;
	PolicyQuotaLocal quota(4*UMMAP_PAGE_SIZE);

	//mapping
	DummyDriver driver1(0);
	FifoWindowPolicy * policy1 = new FifoWindowPolicy(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, true);
	Mapping mapping1(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver1, policy1, NULL);

	//mapping
	DummyDriver driver2(0);
	FifoWindowPolicy * policy2 = new FifoWindowPolicy(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, true);
	Mapping mapping2(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver2, policy2, NULL);

	//register
	quota.registerPolicy(policy1);
	quota.registerPolicy(policy2);

	//touch first on each
	char * ptr1 = (char*)mapping1.getAddress();
	char * ptr2 = (char*)mapping2.getAddress();
	mapping1.onSegmentationFault(ptr1 + 0 * UMMAP_PAGE_SIZE, true);
	mapping2.onSegmentationFault(ptr2 + 0 * UMMAP_PAGE_SIZE, true);
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy1->getCurrentMemory());
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy2->getCurrentMemory());

	//touch many on mapping 2
	mapping2.onSegmentationFault(ptr2 + 1 * UMMAP_PAGE_SIZE, true);
	mapping2.onSegmentationFault(ptr2 + 2 * UMMAP_PAGE_SIZE, true);
	EXPECT_EQ(1*UMMAP_PAGE_SIZE, policy1->getCurrentMemory());
	EXPECT_EQ(3*UMMAP_PAGE_SIZE, policy2->getCurrentMemory());
}
