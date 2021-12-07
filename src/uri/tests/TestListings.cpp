/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../Listings.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestListings, constructor)
{
	Listings listings;
}

/*******************  FUNCTION  *********************/
TEST(TestListings, getObjectId_new)
{
	Listings listings;
	const std::string fname = "test-listings.txt";
	unlink(fname.c_str());
	listings.getObjectId(fname, "test");
}

/*******************  FUNCTION  *********************/
TEST(TestListings, getObjectId_exist)
{
	Listings listings;
	const std::string fname = "test-listings.txt";
	unlink(fname.c_str());
	ObjectId id = listings.getObjectId(fname, "test");
	ObjectId id2 = listings.getObjectId(fname, "test");

	ASSERT_EQ(id.low, id2.low);
	ASSERT_EQ(id.high, id2.high);
}

/*******************  FUNCTION  *********************/
TEST(TestListings, save_restore)
{
	//build
	Listings * listings = new Listings;
	const std::string fname = "test-listings-save.txt";
	unlink(fname.c_str());
	ObjectId id = listings->getObjectId(fname, "test");

	//delete => save
	delete listings;

	//rebuild a new one to load
	Listings * listings2 = new Listings;
	ObjectId id2 = listings2->getObjectId(fname, "test");
	delete listings2;

	//check
	ASSERT_EQ(id.low, id2.low);
	ASSERT_EQ(id.high, id2.high);
}
