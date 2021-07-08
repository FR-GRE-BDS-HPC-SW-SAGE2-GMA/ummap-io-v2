/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "../../portability/OS.hpp"
#include "../MmapDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestMmapDriver, constructor)
{
	MmapDriver driver1;
	MmapDriver driver2(1);
	MmapDriver driver3(1, true);
}

/*******************  FUNCTION  *********************/
TEST(TestMmapDriver, anonymous_no_offset)
{
	MmapDriver driver;
	size_t size = 4* UMMAP_PAGE_SIZE;
	char * ptr = (char*)driver.directMmap(NULL, size, 0, true, true, false, false);
	memset(ptr, 10, size);
	driver.directMunmap(ptr, size, 0);
}

/*******************  FUNCTION  *********************/
TEST(TestMmapDriver, anonymous_offset)
{
	MmapDriver driver;
	size_t size = 4* UMMAP_PAGE_SIZE;
	char * ptr = (char*)driver.directMmap(NULL, size, 4096, true, true, false, false);
	memset(ptr, 10, size);
	driver.directMunmap(ptr, size, 4096);
}

/*******************  FUNCTION  *********************/
TEST(TestMmapDriver, anonymous_not_aligned)
{
	MmapDriver driver;
	size_t size = 4* UMMAP_PAGE_SIZE;
	
	ASSERT_DEATH(driver.directMmap(NULL, size, 1024, true, true, false, false), "aligned");
	ASSERT_DEATH(driver.directMmap(NULL, size+1024, 0, true, true, false, false), "aligned");
}

/*******************  FUNCTION  *********************/
TEST(TestMmapDriver, fileMapping_no_offset)
{
	//open
	const char * fname = "test-no-offset.txt";
	FILE * fp = fopen(fname, "w+");

	//fill
	const size_t size = 4*UMMAP_PAGE_SIZE;
	char buffer[size];
	for (size_t i = 0 ; i < size ; i++)
		buffer[i] = i;
	fwrite(buffer, size, 1, fp);
	fflush(fp);

	//map
	size_t offset = 0;
	MmapDriver driver(fileno(fp));
	char * ptr = (char*)driver.directMmap(NULL, size, offset, true, true, false, false);
	for (size_t i = 0 ; i < size ; i++)
		ASSERT_EQ((char)i, ptr[i]);
	driver.directMunmap(ptr, size, offset);

	//close & unlink
	fclose(fp);
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST(TestMmapDriver, fileMapping_aligned_offset)
{
	//open
	const char * fname = "test-no-offset.txt";
	FILE * fp = fopen(fname, "w+");

	//fill
	const size_t size = 4*UMMAP_PAGE_SIZE;
	char buffer[size];
	for (size_t i = 0 ; i < size ; i++)
		buffer[i] = i + i / UMMAP_PAGE_SIZE;
	fwrite(buffer, size, 1, fp);
	fflush(fp);

	//map
	size_t offset = 4096;
	size_t fsize = size - offset;
	MmapDriver driver(fileno(fp));
	char * ptr = (char*)driver.directMmap(NULL, fsize, offset, true, true, false, false);
	for (size_t i = 0 ; i < fsize ; i++)
		ASSERT_EQ((char)((i + offset)  + (i + offset) / UMMAP_PAGE_SIZE), ptr[i]);
	driver.directMunmap(ptr, fsize, offset);

	//close & unlink
	fclose(fp);
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST(TestMmapDriver, fileMapping_not_aligned_offset)
{
	//open
	const char * fname = "test-no-offset.txt";
	FILE * fp = fopen(fname, "w+");

	//fill
	const size_t size = 4*UMMAP_PAGE_SIZE;
	char buffer[size];
	for (size_t i = 0 ; i < size ; i++)
		buffer[i] = i;
	fwrite(buffer, size, 1, fp);
	fflush(fp);

	//map
	size_t offset = 32;
	size_t fsize = size - offset;
	MmapDriver driver(fileno(fp), true);
	char * ptr = (char*)driver.directMmap(NULL, fsize, offset, true, true, false, false);
	for (size_t i = 0 ; i < fsize ; i++)
		ASSERT_EQ((char)(i+offset), ptr[i]);
	driver.directMunmap(ptr, fsize, offset);

	//close & unlink
	fclose(fp);
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST(TestMmapDriver, sync)
{
	//open
	const char * fname = "/tmp/test-sync.txt";
	const size_t size = 8*4096;
	FILE * fp = fopen(fname, "w+");
	ASSERT_NE(nullptr, fp);
	ASSERT_EQ(0, ftruncate(fileno(fp), size));

	//map
	MmapDriver driver(fileno(fp), true);
	char * ptr = (char*)driver.directMmap(NULL, size, 0, true, true, false, false);
	fclose(fp);

	//write & flush
	memset(ptr, 'a', size);
	driver.directMSync(ptr, 0, size);
	
	//close & unlink
	driver.directMunmap(ptr, size, 0);

	//check content
	fp = fopen(fname, "r");
	ASSERT_NE(nullptr, fp);
	char buffer[size];
	ssize_t res = fread(buffer, 1, size, fp);
	ASSERT_EQ(size, res);
	for (size_t i = 0 ; i < size ; i++)
		ASSERT_EQ('a', buffer[i]) << "Index: " << i;
	fclose(fp);

	//remove
	unlink(fname);
}
