/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//external
#include <gtest/gtest.h>
//internal
#include "../DummyDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestDummyDriver, constructor)
{
	DummyDriver driver1;
	DummyDriver driver2(16);
}

/*******************  FUNCTION  *********************/
TEST(TestDummyDriver, pwrite)
{
	//prep buffer
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	//do write
	DummyDriver driver(16);
	ssize_t res = driver.pwrite(buffer, sizeof(buffer), 0);

	//check status
	ASSERT_EQ(sizeof(buffer), res);

	//check content
	for (int i = 0 ; i < sizeof(buffer) ; i++)
		ASSERT_EQ(0, buffer[i]) << "Index: " << i;
}

/*******************  FUNCTION  *********************/
TEST(TestDummyDriver, pread)
{
	//prep buffer
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	//do write
	DummyDriver driver(16);
	ssize_t res = driver.pread(buffer, sizeof(buffer), 0);

	//check status
	ASSERT_EQ(sizeof(buffer), res);

	//check content
	for (int i = 0 ; i < sizeof(buffer) ; i++)
		ASSERT_EQ(16, buffer[i]) << "Index: " << i;
}
