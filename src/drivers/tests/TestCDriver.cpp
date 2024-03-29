/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
//external
#include <gtest/gtest.h>
//internal
#include "../CDriver.hpp"
#include "../../public-api/ummap.h"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
ssize_t c_dummy_pwrite(void * driver_data, const void * buffer, size_t size, size_t offset)
{
	assert(driver_data != NULL);
	return size;
}

/*******************  FUNCTION  *********************/
ssize_t c_dummy_pread(void * driver_data, void * buffer, size_t size, size_t offset)
{
	assert(driver_data != NULL);
	return size;
}

/*******************  FUNCTION  *********************/
void c_dummy_sync(void * driver_data, void *ptr, size_t offset, size_t size)
{
	assert(driver_data != NULL);
}

/*******************  FUNCTION  *********************/
void * c_dummy_direct_mmap(void * addr, void * driver_data, size_t size, size_t offset, bool read, bool write, bool exec, bool map_fixed)
{
	assert(driver_data != NULL);
	return NULL;
}

/*******************  FUNCTION  *********************/
bool c_dummy_direct_munmap(void * driver_data, void * base, size_t size, size_t offset)
{
	assert(driver_data != NULL);
	return false;
}

/*******************  FUNCTION  *********************/
bool c_dummy_direct_msync(void * driver_data, void * base, size_t size, size_t offset)
{
	assert(driver_data != NULL);
	return false;
}

/*******************  FUNCTION  *********************/
void c_dummy_finalize(void * driver_data)
{
	assert(driver_data != NULL);
	free(driver_data);
}

/********************  STRUCT  **********************/
ummap_c_driver_t gblCDummyDriver = {
	c_dummy_pwrite,
	c_dummy_pread,
	c_dummy_sync,
	c_dummy_direct_mmap,
	c_dummy_direct_munmap,
	c_dummy_direct_msync,
	c_dummy_finalize,
};

/*******************  FUNCTION  *********************/
TEST(TestCDriver, constructor)
{
	void * data = malloc(8);
	CDriver driver(&gblCDummyDriver, data);
}

/*******************  FUNCTION  *********************/
TEST(TestCDriver, functions)
{
	void * data = malloc(8);
	CDriver driver(&gblCDummyDriver, data);
	ASSERT_EQ(10, driver.pwrite(NULL, 10, 0));
	ASSERT_EQ(12, driver.pread(NULL, 12, 0));
	driver.sync(NULL, 0, 0);
	ASSERT_EQ(nullptr, driver.directMmap(NULL, 0, 0, false, false, false, false));
	ASSERT_FALSE(driver.directMunmap(NULL, 0, 0));
	ASSERT_FALSE(driver.directMSync(NULL, 0, 0));
}

/*******************  FUNCTION  *********************/
TEST(TestCDriver, c_api)
{
	//init
	ummap_init();

	//create driver
	ummap_driver_t * driver = ummap_driver_create_c(&gblCDummyDriver, malloc(8));

	//map
	void * ptr = ummap(NULL, 4*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, driver, NULL, NULL);

	//memset
	memset(ptr, 0, 4*4096);

	//unmap
	munmap(ptr, 0);

	//init
	ummap_finalize();
}