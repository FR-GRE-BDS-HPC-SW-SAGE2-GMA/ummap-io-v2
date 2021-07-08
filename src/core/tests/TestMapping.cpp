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
#include "policies/FifoPolicy.hpp"
#include "../Mapping.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;
using namespace testing;

/*******************  FUNCTION  *********************/
TEST(TestMapping, constructor_destructor)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	DummyDriver driver(32);
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//check status
	for (size_t i = 0 ; i < segments ; i++) {
		SegmentStatus status = mapping.getSegmentStatus(i);
		//ASSERT_EQ(0, status.time);
		ASSERT_FALSE(status.dirty);
		ASSERT_FALSE(status.mapped);
		ASSERT_TRUE(status.needRead);
	}
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, map_fixed)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	DummyDriver driver(32);
	void * ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE,0 , 0);
	Mapping mapping(ptr, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_FIXED, &driver, NULL, NULL);

	//check status
	ASSERT_EQ(ptr, mapping.getAddress());
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, first_touch_read_first)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	DummyDriver driver(32);
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//first touch read first segment
	mapping.onSegmentationFault(ptr, false);
	ASSERT_EQ(32, ptr[0]);

	//check status
	SegmentStatus status0 = mapping.getSegmentStatus(0);
	//ASSERT_EQ(0, status0.time);
	ASSERT_FALSE(status0.dirty);
	ASSERT_TRUE(status0.mapped);
	ASSERT_TRUE(status0.needRead);

	//check status
	SegmentStatus status1 = mapping.getSegmentStatus(UMMAP_PAGE_SIZE);
	//ASSERT_EQ(0, status1.time);
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
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//first touch read first segment
	mapping.onSegmentationFault(ptr, true);
	ASSERT_EQ(32, ptr[0]);
	ptr[0] = 64;

	//check status
	SegmentStatus status0 = mapping.getSegmentStatus(0);
	//ASSERT_NE(0, status0.time);
	ASSERT_TRUE(status0.dirty);
	ASSERT_TRUE(status0.mapped);
	ASSERT_TRUE(status0.needRead);

	//check status
	SegmentStatus status1 = mapping.getSegmentStatus(UMMAP_PAGE_SIZE);
	//ASSERT_EQ(0, status1.time);
	ASSERT_FALSE(status1.dirty);
	ASSERT_FALSE(status1.mapped);
	ASSERT_TRUE(status1.needRead);
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, storage_offset)
{
	//setup
	size_t size = 8 * UMMAP_PAGE_SIZE;
	GMockDriver driver;
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 128, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//access
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 128)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	mapping.onSegmentationFault(ptr, false);

	//access
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 128 + UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	mapping.onSegmentationFault(ptr+UMMAP_PAGE_SIZE, false);
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, dropClean)
{
	//setup
	size_t size = 8 * UMMAP_PAGE_SIZE;
	GMockDriver driver;
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 128, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//access
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 128)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	mapping.onSegmentationFault(ptr, false);

	//access
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 128 + UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	mapping.onSegmentationFault(ptr+UMMAP_PAGE_SIZE, true);

	//drop
	mapping.dropClean();

	//access again
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 128)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	mapping.onSegmentationFault(ptr, false);
	mapping.onSegmentationFault(ptr+UMMAP_PAGE_SIZE, false);
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, storage_offset_and_non_full)
{
	//setup
	size_t size = 512;
	GMockDriver driver;
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 128, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//access
	EXPECT_CALL(driver, pread(_, 512, 128)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	mapping.onSegmentationFault(ptr, false);
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, first_last_non_full)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE + 1024;
	GMockDriver driver;
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

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
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

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
	mapping.flush(false);

	//cannot access anymore
	ASSERT_DEATH(ptr[0] = 10, "");
	ASSERT_DEATH(ptr[UMMAP_PAGE_SIZE] = 10, "");
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, sync)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	GMockDriver driver;
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

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
	EXPECT_CALL(driver, sync(ptr, 0, size)).Times(1);

	//flush
	mapping.flush(true);

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
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, localPolicy, &globalPolicy);
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

TEST(TestMapping, globalPolicy)
{
	//setup
	size_t segments = 8;
	size_t size = segments * UMMAP_PAGE_SIZE;
	DummyDriver driver(32);

	//setup local policy
	GMockPolicy * localPolicy1 = new GMockPolicy;
	EXPECT_CALL(*localPolicy1, allocateElementStorage(_, 8));
	GMockPolicy * localPolicy2 = new GMockPolicy;
	EXPECT_CALL(*localPolicy2, allocateElementStorage(_, 8));

	//setup global policy
	FifoPolicy globalPolicy(2*UMMAP_PAGE_SIZE, false);
	
	//create
	Mapping mapping1(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, localPolicy1, &globalPolicy);
	char * ptr1 = (char*)mapping1.getAddress();
	Mapping mapping2(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, localPolicy2, &globalPolicy);
	char * ptr2 = (char*)mapping2.getAddress();

	//touch read first segment
	EXPECT_CALL(*localPolicy1, notifyTouch(&mapping1, 1, false, false, false));
	mapping1.onSegmentationFault(ptr1+UMMAP_PAGE_SIZE, false);

	//touch read first segment
	EXPECT_CALL(*localPolicy2, notifyTouch(&mapping2, 1, false, false, false));
	mapping2.onSegmentationFault(ptr2+UMMAP_PAGE_SIZE, false);

	//touch another which might generate flish
	EXPECT_CALL(*localPolicy2, notifyTouch(&mapping2, 2, false, false, false));
	EXPECT_CALL(*localPolicy1, notifyEvict(_,1));
	mapping2.onSegmentationFault(ptr2+2*UMMAP_PAGE_SIZE, false);

	//expect free
	EXPECT_CALL(*localPolicy1, freeElementStorage(_));
	EXPECT_CALL(*localPolicy2, freeElementStorage(_));
}

/*******************  FUNCTION  *********************/
TEST(TestMapping, copyToDriver_aligned)
{
	//setup
	size_t segments = 4;
	size_t size = segments * UMMAP_PAGE_SIZE;
	GMockDriver driver;
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 2*UMMAP_PAGE_SIZE, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//touch on mapping && we should see two read
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 2*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 3*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 4*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 5*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	for (size_t i = 0 ; i < segments ; i++)
		mapping.onSegmentationFault(ptr + i * UMMAP_PAGE_SIZE, false);

	//touch in write one of the page
	mapping.onSegmentationFault(ptr + 2 * UMMAP_PAGE_SIZE, true);

	//spawn a new driver
	GMockDriver newDriver;

	//expect read from first part of the mapping
	//EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 0*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	//EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 1*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));

	//expect read of the write touched page
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 4*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));

	//expect read from the part after the segment
	//EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 6*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	//EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 7*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));

	//expect write of all
	EXPECT_CALL(newDriver, pwrite(_, UMMAP_PAGE_SIZE, _)).Times(4).WillRepeatedly(Return(UMMAP_PAGE_SIZE));

	//call
	mapping.copyToDriver(&newDriver, 8*UMMAP_PAGE_SIZE);
}

TEST(TestMapping, copyToDriver_not_aligned)
{
	//setup
	size_t segments = 4;
	size_t size = segments * UMMAP_PAGE_SIZE + 1024;
	GMockDriver driver;
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 2*UMMAP_PAGE_SIZE-512, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//get
	char * ptr = (char*)mapping.getAddress();

	//touch on mapping && we should see two read
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 2*UMMAP_PAGE_SIZE-512)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 3*UMMAP_PAGE_SIZE-512)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 4*UMMAP_PAGE_SIZE-512)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 5*UMMAP_PAGE_SIZE-512)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	EXPECT_CALL(driver, pread(_, 1024, 6*UMMAP_PAGE_SIZE-512)).Times(1).WillOnce(Return(1024));
	for (size_t i = 0 ; i < segments ; i++)
		mapping.onSegmentationFault(ptr + i * UMMAP_PAGE_SIZE, false);
	mapping.onSegmentationFault(ptr + segments * UMMAP_PAGE_SIZE + 512, false);

	//touch in write one of the page
	mapping.onSegmentationFault(ptr + 2 * UMMAP_PAGE_SIZE, true);

	//spawn a new driver
	GMockDriver newDriver;

	//expect read from first part of the mapping
	//EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 0*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	//EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE-512, 1*UMMAP_PAGE_SIZE)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE-512));

	//expect read of the write touched page
	EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 4*UMMAP_PAGE_SIZE-512)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));

	//expect read from the part after the segment
	//EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE, 6*UMMAP_PAGE_SIZE+512)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE));
	//EXPECT_CALL(driver, pread(_, UMMAP_PAGE_SIZE-512, 7*UMMAP_PAGE_SIZE+512)).Times(1).WillOnce(Return(UMMAP_PAGE_SIZE-512));

	//expect write of all
	EXPECT_CALL(newDriver, pwrite(_, UMMAP_PAGE_SIZE, _)).Times(4).WillRepeatedly(Return(UMMAP_PAGE_SIZE));
	//EXPECT_CALL(newDriver, pwrite(_, UMMAP_PAGE_SIZE-512, _)).Times(2).WillRepeatedly(Return(UMMAP_PAGE_SIZE-512));
	EXPECT_CALL(newDriver, pwrite(_, 1024, _)).Times(1).WillRepeatedly(Return(1024));

	//call
	//mapping.copyToDriver(&newDriver, 2*UMMAP_PAGE_SIZE-512 + size);
	mapping.copyToDriver(&newDriver, 8*UMMAP_PAGE_SIZE);
}
