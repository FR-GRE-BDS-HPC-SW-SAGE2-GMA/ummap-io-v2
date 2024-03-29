/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../Uri.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestURI, constructor)
{
	Uri uri("file://filename.raw");
	ASSERT_EQ("file://filename.raw", uri.getURI());
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_basic_1)
{
	Uri uri("file://filename.raw");
	ASSERT_EQ("file", uri.getType());
	ASSERT_EQ("filename.raw", uri.getPath());
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_basic_2)
{
	Uri uri("file://directory/filename.raw");
	ASSERT_EQ("file", uri.getType());
	ASSERT_EQ("directory/filename.raw", uri.getPath());
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_params_1)
{
	Uri uri("file://directory/filename.raw?mode=w+");
	ASSERT_EQ("file", uri.getType());
	ASSERT_EQ("directory/filename.raw", uri.getPath());
	ASSERT_EQ("w+", uri.getParam("mode"));
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_params_2)
{
	Uri uri("file://directory/filename.raw?mode=w+&size=1024");
	ASSERT_EQ("file", uri.getType());
	ASSERT_EQ("directory/filename.raw", uri.getPath());
	ASSERT_EQ("w+", uri.getParam("mode"));
	ASSERT_EQ("1024", uri.getParam("size"));
}

/*******************  FUNCTION  *********************/
TEST(TestURI, parse_epxected_uris)
{
	//file
	Uri uri_file("file://directory/filename.raw?mode=w+&size=1024");

	//mero
	Uri uri_mero1("mero://1234:5678");
	Uri uri_mero2("mero://1234:5678?autoinit=true");

	//memory
	Uri uri_mem("mem://10241024");

	//memory
	Uri uri_dummy("dummy://16");
}

TEST(TestURI, getParam_multiple)
{
	Uri uri("mero://auto?listing=listing.txt&name=test&last=ok");

	ASSERT_EQ("listing.txt", uri.getParam("listing"));
	ASSERT_EQ("test", uri.getParam("name"));
	ASSERT_EQ("ok", uri.getParam("last"));
}

TEST(TestURI, getParam)
{
	Uri uri("file://fname?mode=w+");
	ASSERT_EQ("w+", uri.getParam("mode"));
	ASSERT_EQ("w+", uri.getParam("mode", "default"));
	ASSERT_EQ("default", uri.getParam("other", "default"));
	ASSERT_DEATH(uri.getParam("none"), "Fail to find");
}

TEST(TestURI, getParamAsInt)
{
	Uri uri("file://fname?mode=10");
	ASSERT_EQ(10, uri.getParamAsInt("mode"));
	ASSERT_EQ(10, uri.getParamAsInt("mode", 15));
	ASSERT_EQ(20, uri.getParamAsInt("other", 20));
	ASSERT_DEATH(uri.getParamAsInt("none"), "Fail to find");
}

TEST(TestURI, getParamAsSizet)
{
	Uri uri("file://fname?mode=10");
	ASSERT_EQ(10, uri.getParamAsSizet("mode"));
	ASSERT_EQ(10, uri.getParamAsSizet("mode", 25));
	ASSERT_EQ(20, uri.getParamAsSizet("other", 20));
	ASSERT_DEATH(uri.getParamAsSizet("none"), "Fail to find");
}

TEST(TestURI, constructor_invalid)
{
	ASSERT_DEATH(Uri uri("blabla"), "Unrecongnized ummap URI format");
}