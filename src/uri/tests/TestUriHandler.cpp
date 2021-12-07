/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../UriHandler.hpp"
#include "../../policies/FifoPolicy.hpp"
#include "../../policies/LifoPolicy.hpp"
#include "../../policies/FifoWindowPolicy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestUriHandler, constructor)
{
	UriHandler handler;
}

/*******************  FUNCTION  *********************/
TEST(TestUriHandler, replaceVariables)
{
	UriHandler handler;
	handler.registerVariable("fname", "resultat.raw");
	handler.registerVariable("rank", 20);
	handler.registerVariable("rank_long", (size_t)40);

	ASSERT_EQ("file://resultat.raw", handler.replaceVariables("file://{fname}"));
	ASSERT_EQ("file://resultat-20.raw", handler.replaceVariables("file://resultat-{rank}.raw"));
	ASSERT_EQ("file://resultat-40.raw", handler.replaceVariables("file://resultat-{rank_long}.raw"));
}

/*******************  FUNCTION  *********************/
TEST(TestUriHandler, buildDriver)
{
	//var
	Driver * driver = NULL;
	UriHandler handler;

	//file
	driver = handler.buildDriver("file://test.raw");
	delete driver;

	//mem
	driver = handler.buildDriver("mem://4096");
	delete driver;

	//dummy
	driver = handler.buildDriver("dummy://0");
	delete driver;

	//mmap
	driver = handler.buildDriver("mmap://test.raw");
	delete driver;

	//mmap
	driver = handler.buildDriver("mmap://test.raw?mode=w+");
	delete driver;

	//mmap
	driver = handler.buildDriver("dax://test.raw");
	delete driver;

	//mero-1 
	//driver = handler.buildDriver("merofile://123:456");
	//delete driver;

	//mero-2
	//driver = handler.buildDriver("merofile://auto?listing=listing.txt&name=test");
	//delete driver;
}

/*******************  FUNCTION  *********************/
TEST(TestUriHandler, buildPolicy)
{
	//var
	Policy * policy = NULL;
	UriHandler handler;

	//fifo
	policy = handler.buildPolicy("fifo://4096", true);
	ASSERT_NE(nullptr, dynamic_cast<FifoPolicy*>(policy));
	delete policy;

	//fifo
	policy = handler.buildPolicy("fifo-window://8MB?window=1MB", true);
	ASSERT_NE(nullptr, dynamic_cast<FifoWindowPolicy*>(policy));
	delete policy;

	//lifo
	policy = handler.buildPolicy("lifo://4096", true);
	ASSERT_NE(nullptr, dynamic_cast<LifoPolicy*>(policy));
	delete policy;
}
