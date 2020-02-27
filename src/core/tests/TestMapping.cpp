/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../../portability/OS.hpp"
#include "../../drivers/DummyDriver.hpp"
#include "../Mapping.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*********************  CLASS  **********************/
class TestMapping : public testing::Test
{
	public:
		virtual void SetUp() override;
		virtual void TearDown() override;
	protected:
		size_t segments;
		size_t size;
		Driver * driver;
		Mapping * mapping;
};

/*******************  FUNCTION  *********************/
void TestMapping::SetUp()
{
	this->segments = 8;
	this->size = segments * UMMAP_PAGE_SIZE;
	this->driver = new DummyDriver(32);
	this->mapping = new Mapping(size, UMMAP_PAGE_SIZE, MAPPING_PROT_RW, driver, NULL, NULL);
}

/*******************  FUNCTION  *********************/
void TestMapping::TearDown()
{
	delete this->mapping;
	delete this->driver;
}

/*******************  FUNCTION  *********************/
TEST_F(TestMapping, constructor_destructor)
{
	for (size_t i = 0 ; i < this->segments ; i++) {
		SegmentStatus status = this->mapping->getSegmentStatus(i);
		ASSERT_EQ(0, status.time);
		ASSERT_FALSE(status.dirty);
		ASSERT_FALSE(status.mapped);
		ASSERT_TRUE(status.needRead);
	}
}

/*******************  FUNCTION  *********************/
TEST_F(TestMapping, first_touch_read_first)
{
	//get
	char * ptr = (char*)mapping->getAddress();

	//first touch read first segment
	mapping->onSegmentationFault(ptr, false);
	ASSERT_EQ(32, ptr[0]);

	//check status
	SegmentStatus status0 = this->mapping->getSegmentStatus(0);
	ASSERT_EQ(0, status0.time);
	ASSERT_FALSE(status0.dirty);
	ASSERT_TRUE(status0.mapped);
	ASSERT_TRUE(status0.needRead);

	//check status
	SegmentStatus status1 = this->mapping->getSegmentStatus(UMMAP_PAGE_SIZE);
	ASSERT_EQ(0, status1.time);
	ASSERT_FALSE(status1.dirty);
	ASSERT_FALSE(status1.mapped);
	ASSERT_TRUE(status1.needRead);
}

/*******************  FUNCTION  *********************/
TEST_F(TestMapping, first_touch_write_first)
{
	//get
	char * ptr = (char*)mapping->getAddress();

	//first touch read first segment
	mapping->onSegmentationFault(ptr, true);
	ASSERT_EQ(32, ptr[0]);
	ptr[0] = 64;

	//check status
	SegmentStatus status0 = this->mapping->getSegmentStatus(0);
	ASSERT_NE(0, status0.time);
	ASSERT_TRUE(status0.dirty);
	ASSERT_TRUE(status0.mapped);
	ASSERT_TRUE(status0.needRead);

	//check status
	SegmentStatus status1 = this->mapping->getSegmentStatus(UMMAP_PAGE_SIZE);
	ASSERT_EQ(0, status1.time);
	ASSERT_FALSE(status1.dirty);
	ASSERT_FALSE(status1.mapped);
	ASSERT_TRUE(status1.needRead);
}
