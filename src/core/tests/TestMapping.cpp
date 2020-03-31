/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "portability/OS.hpp"
#include "drivers/DummyDriver.hpp"
#include "drivers/GMockDriver.hpp"
#include "policies/GMockPolicy.hpp"
#include "../Mapping.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;
using namespace testing;

/*******************  FUNCTION  *********************/
TEST(TestMapping, constructor_destructor)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	DummyDriver driver(32);
	Mapping mapping(size, UMMAP_PAGE_SIZE, MAPPING_PROT_RW, &driver, NULL, NULL);

	//check status
	for (size_t i = 0 ; i < segments ; i++) {
		SegmentStatus status = mapping.getSegmentStatus(i);
		ASSERT_EQ(0, status.time);
		ASSERT_FALSE(status.dirty);
		ASSERT_FALSE(status.mapped);
		ASSERT_TRUE(status.needRead);
	}
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, first_touch_read_first)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	DummyDriver driver(32);
	Mapping mapping(size, UMMAP_PAGE_SIZE, MAPPING_PROT_RW, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//first touch read first segment
	mapping.onSegmentationFault(ptr, false);
	ASSERT_EQ(32, ptr[0]);

	//check status
	SegmentStatus status0 = mapping.getSegmentStatus(0);
	ASSERT_EQ(0, status0.time);
	ASSERT_FALSE(status0.dirty);
	ASSERT_TRUE(status0.mapped);
	ASSERT_TRUE(status0.needRead);

	//check status
	SegmentStatus status1 = mapping.getSegmentStatus(UMMAP_PAGE_SIZE);
	ASSERT_EQ(0, status1.time);
	ASSERT_FALSE(status1.dirty);
	ASSERT_FALSE(status1.mapped);
	ASSERT_TRUE(status1.needRead);
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, first_touch_write_first)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	DummyDriver driver(32);
	Mapping mapping(size, UMMAP_PAGE_SIZE, MAPPING_PROT_RW, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//first touch read first segment
	mapping.onSegmentationFault(ptr, true);
	ASSERT_EQ(32, ptr[0]);
	ptr[0] = 64;

	//check status
	SegmentStatus status0 = mapping.getSegmentStatus(0);
	ASSERT_NE(0, status0.time);
	ASSERT_TRUE(status0.dirty);
	ASSERT_TRUE(status0.mapped);
	ASSERT_TRUE(status0.needRead);

	//check status
	SegmentStatus status1 = mapping.getSegmentStatus(UMMAP_PAGE_SIZE);
	ASSERT_EQ(0, status1.time);
	ASSERT_FALSE(status1.dirty);
	ASSERT_FALSE(status1.mapped);
	ASSERT_TRUE(status1.needRead);
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, first_last_non_full)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE + 1024;
	GMockDriver driver;
	Mapping mapping(size, UMMAP_PAGE_SIZE, MAPPING_PROT_RW, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//access
	EXPECT_CALL(driver, pread(_, 1024, segments * UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	mapping.onSegmentationFault(ptr + size - 10, false);
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, flush)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	GMockDriver driver;
	Mapping mapping(size, UMMAP_PAGE_SIZE, MAPPING_PROT_RW, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//we should see two read
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 0)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));

	//touch to map
	mapping.onSegmentationFault(ptr + 0 * UMMAP_PAGE_SIZE, true);
	mapping.onSegmentationFault(ptr + 1 * UMMAP_PAGE_SIZE, true);

	//set values
	for (size_t i = 0 ; i < 2 * UMMAP_PAGE_SIZE ; i++) 
		ptr[i] = 64;

	//we should see two write
	EXPECT_CALL(driver, pwrite(_, UMMAP_PAGE_SIZE, 0)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	EXPECT_CALL(driver, pwrite(_, UMMAP_PAGE_SIZE, UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));

	//flush
	mapping.flush();

	//cannot access anymore
	ASSERT_DEATH(ptr[0] = 10, "");
	ASSERT_DEATH(ptr[UMMAP_PAGE_SIZE] = 10, "");
}

TEST(TestMapping, policy)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	DummyDriver driver(32);

	//setup local policy
	GMockPolicy * localPolicy = new GMockPolicy;
	EXPECT_CALL(*localPolicy, allocateElementStorage(_, 8));

	//setup global policy
	GMockPolicy globalPolicy;
	EXPECT_CALL(globalPolicy, allocateElementStorage(_, 8));
	
	//create
	Mapping mapping(size, UMMAP_PAGE_SIZE, MAPPING_PROT_RW, &driver, localPolicy, &globalPolicy);
	char * ptr = (char*)mapping.getAddress();

	//touch read
	EXPECT_CALL(*localPolicy, notifyTouch(&mapping, 1, false, false, false));
	EXPECT_CALL(globalPolicy, notifyTouch(&mapping, 1, false, false, false));
	mapping.onSegmentationFault(ptr+UMMAP_PAGE_SIZE, false);
	
	//touch write
	EXPECT_CALL(*localPolicy, notifyTouch(&mapping, 1, true, true, false));
	EXPECT_CALL(globalPolicy, notifyTouch(&mapping, 1, true, true, false));
	mapping.onSegmentationFault(ptr+UMMAP_PAGE_SIZE, true);

	//check not again
	mapping.onSegmentationFault(ptr+UMMAP_PAGE_SIZE, true);

	//expect free
	EXPECT_CALL(*localPolicy, freeElementStorage(_));
	EXPECT_CALL(globalPolicy, freeElementStorage(_));
}