/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "policies/FifoPolicy.hpp"
#include "../PolicyRegistry.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestPolicyRegistry, constructor)
{
	PolicyRegistry registry;
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyRegistry, register_get)
{
	PolicyRegistry registry;
	Policy * policy = new FifoPolicy(4096, false);
	registry.registerPolicy("test", policy);
	ASSERT_EQ(policy, registry.get("test"));
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyRegistry, get_null)
{
	PolicyRegistry registry;
	ASSERT_EQ(nullptr, registry.get("test", true));
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyRegistry, unregister)
{
	PolicyRegistry registry;
	Policy * policy = new FifoPolicy(4096, false);
	registry.registerPolicy("test", policy);
	ASSERT_EQ(policy, registry.get("test"));
	registry.unregisterPolicy("test");
	ASSERT_EQ(nullptr, registry.get("test", true));
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyRegistry, isEmpty)
{
	PolicyRegistry registry;
	ASSERT_TRUE(registry.isEmpty());
	Policy * policy = new FifoPolicy(4096, false);
	registry.registerPolicy("test", policy);
	ASSERT_FALSE(registry.isEmpty());
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyRegistry, deleteAll)
{
	PolicyRegistry registry;
	ASSERT_TRUE(registry.isEmpty());
	Policy * policy = new FifoPolicy(4096, false);
	registry.registerPolicy("test", policy);
	ASSERT_FALSE(registry.isEmpty());
	registry.deleteAll();
	ASSERT_TRUE(registry.isEmpty());
}
