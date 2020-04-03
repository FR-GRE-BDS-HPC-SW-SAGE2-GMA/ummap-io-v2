/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//gtest
#include <gtest/gtest.h>
//local
#include "../MemoryDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestMemoryDriver, constructor)
{
	MemoryDriver driver(1024*1024, 32);
}

/*******************  FUNCTION  *********************/
TEST(TestMemoryDriver, pread)
{
	MemoryDriver driver(1024*1024, 32);
	char buffer[1024];
	driver.pread(buffer, sizeof(buffer), 0);
	for (size_t i = 0 ; i < sizeof(buffer) ; i++)
		ASSERT_EQ(32, buffer[i]);
}

/*******************  FUNCTION  *********************/
TEST(TestMemoryDriver, pwrite)
{
	MemoryDriver driver(1024*1024, 32);

	//set
	char buffer[1024];
	memset(buffer, 48, sizeof(buffer));

	//fill
	driver.pwrite(buffer, sizeof(buffer), 0);

	//check
	const char * internalBuffer = driver.getBuffer();
	for (size_t i = 0 ; i < sizeof(buffer) ; i++)
		ASSERT_EQ(48, internalBuffer[i]);
}

/*******************  FUNCTION  *********************/
TEST(TestMemoryDriver, sync)
{
	MemoryDriver driver(1024*1024, 32);
	driver.sync(0, 1024*1024);
}

/*******************  FUNCTION  *********************/
TEST(TestMemoryDriver, getSize)
{
	MemoryDriver driver(1024*1024, 32);
	ASSERT_EQ(1024*1024, driver.getSize());
}

/*******************  FUNCTION  *********************/
TEST(TestMemoryDriver, dup)
{
	MemoryDriver driver(1024*1024, 32);
	MemoryDriver * dup = new MemoryDriver(&driver);

	//set
	for (int i = 0 ; i < 1024*1024; i++)
		driver.getBuffer()[i] = (char)i;

	//check on copy
	for (int i = 0 ; i < 1024*1024; i++)
		ASSERT_EQ((char)i, dup->getBuffer()[i]) << "Index : " << i;

	//delete
	delete dup;
}
