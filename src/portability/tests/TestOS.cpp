/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/*******************  FUNCTION  *********************/
#include <gtest/gtest.h>
#include "../OS.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
TEST(TestOS, mmapProtNone)
{
	const size_t size = 10*UMMAP_PAGE_SIZE;
	char * ptr = (char*)OS::mmapProtNone(size);
	ASSERT_NE(ptr, nullptr);
	ASSERT_DEATH(ptr[0] = 'a', "");
	OS::munmap(ptr, size);
}

/*******************  FUNCTION  *********************/
TEST(TestOS, cpuNumber)
{
	int status = OS::cpuNumber();
	ASSERT_GT(status, 0);
}
