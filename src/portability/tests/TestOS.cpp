/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/*******************  FUNCTION  *********************/
#include <gtest/gtest.h>
#include "../OS.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestOS, mmapProtFull)
{
	const size_t size = 10*UMMAP_PAGE_SIZE;
	char * ptr = (char*)OS::mmapProtFull(size, false);
	ASSERT_NE(ptr, nullptr);
	for (size_t i = 0 ; i < size ; i++)
		ASSERT_EQ(0, ptr[i]) << "Index: " << i;
	OS::munmap(ptr, size);
}

/*******************  FUNCTION  *********************/
TEST(TestOS, mmapProtNone)
{
	const size_t size = 10*UMMAP_PAGE_SIZE;
	char * ptr = (char*)OS::mmapProtNone(NULL, size, false);
	ASSERT_NE(ptr, nullptr);
	ASSERT_DEATH(ptr[0] = 'a', "");
	OS::munmap(ptr, size);
}

/*******************  FUNCTION  *********************/
TEST(TestOS, mremapForced)
{
	const size_t size = 10*UMMAP_PAGE_SIZE;

	//map
	char * ptr = (char*)OS::mmapProtFull(2*size, false);
	ASSERT_NE(ptr, nullptr);
	
	//remap
	char * newPtrExpect = ptr + size;
	OS::mremapForced(ptr, size, newPtrExpect);
	
	//not access to old one
	ASSERT_DEATH(ptr[0] = 'a', "");

	//check access
	for (size_t i = 0 ; i < size ; i++)
		ASSERT_EQ(0, newPtrExpect[i]) << "Index: " << i;

	OS::munmap(newPtrExpect, size);
}

/*******************  FUNCTION  *********************/
TEST(TestOS, mprotect_read_only)
{
	const size_t size = 10*UMMAP_PAGE_SIZE;

	//map
	char * ptr = (char*)OS::mmapProtNone(NULL, size, false);

	//protect
	OS::mprotect(ptr, size, true, false, false);

	//check access
	for (size_t i = 0 ; i < size ; i++)
		ASSERT_EQ(0, ptr[i]) << "Index: " << i;

	//not access to old one
	ASSERT_DEATH(ptr[0] = 'a', "");
}

/*******************  FUNCTION  *********************/
TEST(TestOS, mprotect_read_write)
{
	const size_t size = 10*UMMAP_PAGE_SIZE;

	//map
	char * ptr = (char*)OS::mmapProtNone(NULL, size, false);

	//protect
	OS::mprotect(ptr, size, true, true, false);

	//check access
	for (size_t i = 0 ; i < size ; i++) {
		ASSERT_EQ(0, ptr[i]) << "Index: " << i;
		ptr[0] = 'a';
	}
}

/*******************  FUNCTION  *********************/
TEST(TestOS, cpuNumber)
{
	int status = OS::cpuNumber();
	ASSERT_GT(status, 0);
}
