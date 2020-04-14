/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../HumanUnits.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestHumanUnits, basic)
{
	ASSERT_EQ(32, fromHumanMemSize("32"));
	ASSERT_EQ(32, fromHumanMemSize("32B"));
}

TEST(TestHumanUnits, kb)
{
	ASSERT_EQ(32*1024, fromHumanMemSize("32KB"));
}

TEST(TestHumanUnits, mb)
{	
	ASSERT_EQ(32*1024*1024, fromHumanMemSize("32MB"));
}

TEST(TestHumanUnits, gb)
{
	ASSERT_EQ(32*1024ul*1024ul*1024ul, fromHumanMemSize("32GB"));
}

TEST(TestHumanUnits, tb)
{
	ASSERT_EQ(32*1024ul*1024ul*1024ul*1024ul, fromHumanMemSize("32TB"));
}
