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

	//unmap 1 and let the other for cleaup
	umunmap(ptr1);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_unmap_multi)
{
	//map 2
	void * ptr1 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_dummy(16), NULL, "none");
	void * ptr2 = ummap(8*4096, 4096, 0, UMMAP_PROT_RW, ummap_driver_create_dummy(16), NULL, "none");

	//unmap 1 and let the other for cleaup
	umunmap(ptr1);
}
