/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//google test
#include <gtest/gtest.h>
//internal
#include "../../core/GMockMapping.hpp"
#include "../../portability/OS.hpp"
#include "../../drivers/DummyDriver.hpp"
#include "../GMockPolicy.hpp"
//local
#include "../LifoPolicy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;
using namespace testing;

/*********************  CLASS  **********************/
class MockLifoPolicy : public LifoPolicy
{
	public:
		MockLifoPolicy(size_t maxMemory, bool local) : LifoPolicy(maxMemory, local) {};
		size_t getCurrentMemory(void) {return this->currentMemory;};
};

/*******************  FUNCTION  *********************/
TEST(TestLifoPolicy, constructor)
{
	LifoPolicy policy(10*1024*1024, true);
}

/*******************  FUNCTION  *********************/
TEST(TestLifoPolicy, reg)
{
	//set
	LifoPolicy policy(2*UMMAP_PAGE_SIZE, true);
	DummyDriver driver;
	Mapping mapping(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, &driver, NULL, NULL);

	policy.allocateElementStorage(&mapping, 8);
	policy.freeElementStorage(&mapping);
}

/*******************  FUNCTION  *********************/
TEST(TestLifoPolicy, evict)
{
	//set
	LifoPolicy * policy = new LifoPolicy(2*UMMAP_PAGE_SIZE, true);
	DummyDriver driver;
	GMockMapping mapping(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, &driver, policy, NULL);
	char * ptr = (char*)mapping.getAddress();

	//touch no effect
	mapping.onSegmentationFault(ptr+0*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+1*UMMAP_PAGE_SIZE, true);

	//touch effect 0
	EXPECT_CALL(mapping, evict(policy, 1));
	mapping.onSegmentationFault(ptr+2*UMMAP_PAGE_SIZE, true);

	//touch effect 1
	EXPECT_CALL(mapping, evict(policy, 2));
	mapping.onSegmentationFault(ptr+3*UMMAP_PAGE_SIZE, true);

	//touch effect 2
	EXPECT_CALL(mapping, evict(policy, 3));
	mapping.onSegmentationFault(ptr+4*UMMAP_PAGE_SIZE, true);
}

/*******************  FUNCTION  *********************/
TEST(TestLifoPolicy, evict_notify_other)
{
	//set
	LifoPolicy * policy = new LifoPolicy(2*UMMAP_PAGE_SIZE, true);
	GMockPolicy globalPolicy;
	EXPECT_CALL(globalPolicy, allocateElementStorage(_, 8));
	EXPECT_CALL(globalPolicy, freeElementStorage(_));

	//setup
	DummyDriver driver;
	Mapping mapping(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, &driver, policy, &globalPolicy);
	char * ptr = (char*)mapping.getAddress();

	//touch no effect
	EXPECT_CALL(globalPolicy, notifyTouch(&mapping, _, true, false, false)).Times(3);
	mapping.onSegmentationFault(ptr+0*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+1*UMMAP_PAGE_SIZE, true);

	//touch effect 0
	EXPECT_CALL(globalPolicy, notifyEvict(&mapping, 1));
	mapping.onSegmentationFault(ptr+2*UMMAP_PAGE_SIZE, true);
}

/*******************  FUNCTION  *********************/
TEST(TestLifoPolicy, freeElementStorage)
{
	//set
	MockLifoPolicy policy(2*UMMAP_PAGE_SIZE, true);

	//setup
	DummyDriver driver;
	Mapping * mapping = new Mapping(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, &driver, NULL, &policy);
	char * ptr = (char*)mapping->getAddress();

	//check
	ASSERT_EQ(0, policy.getCurrentMemory());

	//touch
	mapping->onSegmentationFault(ptr+0*UMMAP_PAGE_SIZE, true);
	mapping->onSegmentationFault(ptr+1*UMMAP_PAGE_SIZE, true);

	//check
	ASSERT_EQ(2*UMMAP_PAGE_SIZE, policy.getCurrentMemory());

	//remove
	delete mapping;

	//check
	ASSERT_EQ(0, policy.getCurrentMemory());
}
