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
#include "../FifoWindowPolicy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;
using namespace testing;

/*********************  CLASS  **********************/
class MockFifoWindowPolicy : public FifoWindowPolicy
{
	public:
		MockFifoWindowPolicy(size_t maxMemory, size_t slidingWindow, bool local) : FifoWindowPolicy(maxMemory, slidingWindow, local) {};
		size_t getCurrentFixedMemory(void) {return this->currentFixedMemory;};
		size_t getCurrentSlidingWindowMemory(void) {return this->currentSlidingWindowMemory;};
};

/*******************  FUNCTION  *********************/
TEST(TestFifoWindowPolicy, constructor)
{
	FifoWindowPolicy policy(10*1024*1024, 1024*1024, true);
}

/*******************  FUNCTION  *********************/
TEST(TestFifoWindowPolicy, reg)
{
	//set
	FifoWindowPolicy policy(4*UMMAP_PAGE_SIZE, 2*UMMAP_PAGE_SIZE, true);
	DummyDriver driver;
	Mapping mapping(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, &driver, NULL, NULL);

	policy.allocateElementStorage(&mapping, 8);
	policy.freeElementStorage(&mapping);
}

/*******************  FUNCTION  *********************/
TEST(TestFifoWindowPolicy, evict)
{
	//set
	FifoWindowPolicy * policy = new FifoWindowPolicy(8*UMMAP_PAGE_SIZE, 2*UMMAP_PAGE_SIZE, true);
	DummyDriver driver;
	GMockMapping mapping(16*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, &driver, policy, NULL);
	char * ptr = (char*)mapping.getAddress();

	//touch no effect
	mapping.onSegmentationFault(ptr+0*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+1*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+2*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+3*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+4*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+5*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+6*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+7*UMMAP_PAGE_SIZE, true);

	//touch effect 0
	EXPECT_CALL(mapping, evict(policy, 6));
	mapping.onSegmentationFault(ptr+8*UMMAP_PAGE_SIZE, true);

	//touch effect 1
	EXPECT_CALL(mapping, evict(policy, 7));
	mapping.onSegmentationFault(ptr+9*UMMAP_PAGE_SIZE, true);

	//touch effect 2
	EXPECT_CALL(mapping, evict(policy, 8));
	mapping.onSegmentationFault(ptr+10*UMMAP_PAGE_SIZE, true);

	//touch no effect
	mapping.onSegmentationFault(ptr+0*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+1*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+2*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+3*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+4*UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr+5*UMMAP_PAGE_SIZE, true);
}

/*******************  FUNCTION  *********************/
TEST(TestFifoWindowPolicy, evict_notify_other)
{
	//set
	FifoWindowPolicy * policy = new FifoWindowPolicy(2*UMMAP_PAGE_SIZE, 1*UMMAP_PAGE_SIZE, true);
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
TEST(TestFifoPolicy, freeElementStorage)
{
	//set
	MockFifoWindowPolicy policy(3*UMMAP_PAGE_SIZE,1*UMMAP_PAGE_SIZE, true);

	//setup
	DummyDriver driver;
	Mapping * mapping = new Mapping(8*UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, &driver, NULL, &policy);
	char * ptr = (char*)mapping->getAddress();

	//check
	ASSERT_EQ(0, policy.getCurrentFixedMemory());
	ASSERT_EQ(0, policy.getCurrentSlidingWindowMemory());

	//touch
	mapping->onSegmentationFault(ptr+0*UMMAP_PAGE_SIZE, true);
	mapping->onSegmentationFault(ptr+1*UMMAP_PAGE_SIZE, true);
	mapping->onSegmentationFault(ptr+2*UMMAP_PAGE_SIZE, true);

	//check
	ASSERT_EQ(2*UMMAP_PAGE_SIZE, policy.getCurrentFixedMemory());
	ASSERT_EQ(1*UMMAP_PAGE_SIZE, policy.getCurrentSlidingWindowMemory());

	//remove
	delete mapping;

	//check
	EXPECT_EQ(0, policy.getCurrentFixedMemory());
	EXPECT_EQ(0, policy.getCurrentSlidingWindowMemory());
}
