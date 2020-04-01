/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../URI.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestURI, constructor)
{
	URI uri("file://filename.raw");
	ASSERT_EQ("file://filename.raw", uri.getURI());
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_basic_1)
{
	URI uri("file://filename.raw");
	ASSERT_EQ("file", uri.getType());
	ASSERT_EQ("filename.raw", uri.getPath());
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_basic_2)
{
	URI uri("file://directory/filename.raw");
	ASSERT_EQ("file", uri.getType());
	ASSERT_EQ("directory/filename.raw", uri.getPath());
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_params_1)
{
	URI uri("file://directory/filename.raw?mode=w+");
	ASSERT_EQ("file", uri.getType());
	ASSERT_EQ("directory/filename.raw", uri.getPath());
	ASSERT_EQ("w+", uri.getParam("mode"));
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_params_2)
{
	URI uri("file://directory/filename.raw?mode=w+&size=1024");
	ASSERT_EQ("file", uri.getType());
	ASSERT_EQ("directory/filename.raw", uri.getPath());
	ASSERT_EQ("w+", uri.getParam("mode"));
	ASSERT_EQ("1024", uri.getParam("size"));
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_epxected_uris)
{
	//file
	URI uri_file("file://directory/filename.raw?mode=w+&size=1024");

	//mero
	URI uri_mero1("mero://1234:5678");
	URI uri_mero2("mero://1234:5678?autoinit=true");

	//memory
	URI uri_mem("mem://10241024");

	//memory
	URI uri_dummy("dummy://16");
}
