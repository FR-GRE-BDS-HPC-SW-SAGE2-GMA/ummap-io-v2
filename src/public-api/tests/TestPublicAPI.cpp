/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../ummap.h"

/*********************  CLASS  **********************/
class TestPublicAPI : public testing::Test
{
	public:
		virtual void SetUp(void);
		virtual void TearDown(void);
};

/*******************  FUNCTION  *********************/
void TestPublicAPI::SetUp(void)
{
	ummap_init();
}

/*******************  FUNCTION  *********************/
void TestPublicAPI::TearDown(void)
{
	ummap_finalize();
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, init_finalize)
{
	//just setup/teardown
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, reinit_failure)
{
	//init again
	ASSERT_DEATH(ummap_init(), "re-init");
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_unmap_simple)
{
	//map 2
	void * ptr1 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_dummy(16), NULL, "none");
	memset(ptr1, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_unmap_multi)
{
	//map 2
	void * ptr1 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_dummy(16), NULL, "none");
	void * ptr2 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_dummy(16), NULL, "none");

	//touch
	memset(ptr1, 10, 8*4096);
	memset(ptr2, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_fopen)
{
	//def
	const char * fname = "/tmp/test-ummap-fopen-driver.txt";

	//map 2
	void * ptr1 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_fopen(fname, "w+"), NULL, "none");

	//we just write, no read pre-existing content
	ummap_skip_first_read(ptr1);

	//setup
	memset(ptr1, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1);

	//check
	FILE * fp = fopen(fname, "r");
	ASSERT_NE(nullptr, fp);
	char buffer[8*4096];
	fread(buffer, 1, 8*4096, fp);
	for (int i = 0 ; i < 8*4096 ; i++)
		ASSERT_EQ(10, buffer[i]);
	
	//clear
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_fd)
{
	//def
	const char * fname = "/tmp/test-ummap-fopen-driver.txt";

	//open
	FILE * fp = fopen(fname, "w+");
	ASSERT_NE(nullptr, fp);
	int fd = fileno(fp);

	//map 2
	void * ptr1 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_fd(fd), NULL, "none");
	fclose(fp);

	//we just write, no read pre-existing content
	ummap_skip_first_read(ptr1);

	//setup
	memset(ptr1, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1);

	//check
	fp = fopen(fname, "r");
	ASSERT_NE(nullptr, fp);
	char buffer[8*4096];
	fread(buffer, 1, 8*4096, fp);
	for (int i = 0 ; i < 8*4096 ; i++)
		ASSERT_EQ(10, buffer[i]);
	
	//clear
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_memory_ok)
{
	void * ptr1 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_memory(8*4096), NULL, "none");
	memset(ptr1, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_memory_not_ok)
{
	void * ptr1 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_memory(4*4096), NULL, "none");
	ASSERT_DEATH(memset(ptr1, 10, 8*4096),"overpass");

	//unmap 1 and let the other for cleaup
	umunmap(ptr1);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_dummy)
{
	void * ptr1 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_dummy(64), NULL, "none");
	memset(ptr1, 10, 8*4096);

	//check
	for (int i = 0 ; i < 8*4096 ; i++)
		ASSERT_EQ(10, ((char*)ptr1)[i]);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, destroy_driver)
{
	ummap_driver_t * driver = ummap_driver_create_dummy(64);
	ummap_driver_destroy(driver);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, autoclean_driver)
{
	ummap_driver_t * driver = ummap_driver_create_dummy(64);
	ummap_driver_set_autoclean(driver, false);
	ummap_driver_destroy(driver);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, policy_group)
{
	//setup policies
	ummap_policy_t * globalPolicy = umamp_policy_create_fifo(4*4096, false);
	ummap_policy_t * localPolicy = umamp_policy_create_fifo(4*4096, true);

	//reg global
	ummap_policy_group_register("global", globalPolicy);

	//map
	void * ptr1 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_dummy(64), localPolicy, "global");
	memset(ptr1, 0 , 8*4096);

	//unmap
	umunmap(ptr1);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, lbm_bugguy_case_1)
{
	//set buggy size non multiple of 1MB
	size_t size = 37140768;

	//map
	void * ptr1 = ummap(size, 1024*1024, 0, UMMAP_PROT_RW, ummap_driver_create_dummy(64), NULL, "none");

	//touch
	#pragma parallel
	for (size_t i = 0 ; i < size ; i++)
		((char*)ptr1)[i] = 34;

	//unam
	umunmap(ptr1);
}
