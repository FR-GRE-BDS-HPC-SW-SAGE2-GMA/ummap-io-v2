/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../ummap.h"

/*******************  FUNCTION  *********************/
TEST(TestPublicAPI, init_finalize)
{
	ummap_init();
	ummap_finalize();
}

/*******************  FUNCTION  *********************/
TEST(TestPublicAPI, reinit_failure)
{
	ummap_init();
	ASSERT_DEATH(ummap_init(), "re-init");
	ummap_finalize();
}
