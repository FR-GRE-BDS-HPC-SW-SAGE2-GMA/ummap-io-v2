/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
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

/*******************  FUNCTION  *********************/
TEST(TestHumanUnits, kb)
{
	ASSERT_EQ(32*1024, fromHumanMemSize("32KB"));
}

/*******************  FUNCTION  *********************/
TEST(TestHumanUnits, mb)
{	
	ASSERT_EQ(32*1024*1024, fromHumanMemSize("32MB"));
}

/*******************  FUNCTION  *********************/
TEST(TestHumanUnits, gb)
{
	ASSERT_EQ(32*1024ul*1024ul*1024ul, fromHumanMemSize("32GB"));
}

/*******************  FUNCTION  *********************/
TEST(TestHumanUnits, tb)
{
	ASSERT_EQ(32*1024ul*1024ul*1024ul*1024ul, fromHumanMemSize("32TB"));
}

/*******************  FUNCTION  *********************/
TEST(TestHumanUnits, regexp_version)
{
	ASSERT_EQ(32, fromHumanMemSizeRegexp("32"));
	ASSERT_EQ(32, fromHumanMemSizeRegexp("32B"));
	ASSERT_EQ(32*1024, fromHumanMemSizeRegexp("32KB"));
	ASSERT_EQ(32*1024*1024, fromHumanMemSizeRegexp("32MB"));
	ASSERT_EQ(32*1024ul*1024ul*1024ul, fromHumanMemSizeRegexp("32GB"));
	ASSERT_EQ(32*1024ul*1024ul*1024ul*1024ul, fromHumanMemSizeRegexp("32TB"));
}

/*******************  FUNCTION  *********************/
TEST(TestHumanUnits, no_regexp_version)
{
	ASSERT_EQ(32, fromHumanMemSizeNoRegexp("32"));
	ASSERT_EQ(32, fromHumanMemSizeNoRegexp("32B"));
	ASSERT_EQ(32*1024, fromHumanMemSizeNoRegexp("32KB"));
	ASSERT_EQ(32*1024*1024, fromHumanMemSizeNoRegexp("32MB"));
	ASSERT_EQ(32*1024ul*1024ul*1024ul, fromHumanMemSizeNoRegexp("32GB"));
	ASSERT_EQ(32*1024ul*1024ul*1024ul*1024ul, fromHumanMemSizeNoRegexp("32TB"));
}
