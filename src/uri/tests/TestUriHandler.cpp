/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../UriHandler.hpp"

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

	ASSERT_EQ("file://resultat.raw", handler.replaceVariables("file://{fname}"));
	ASSERT_EQ("file://resultat-20.raw", handler.replaceVariables("file://resultat-{rank}.raw"));
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
}

/*******************  FUNCTION  *********************/
TEST(TestUriHandler, buildPolicy)
{
	//var
	Policy * policy = NULL;
	UriHandler handler;

	//firo
	policy = handler.buildPolicy("fifo://4096", true);
	delete policy;
}
