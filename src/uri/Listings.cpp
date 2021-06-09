/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstdio>
#include <cstdlib>
#include <ctime>
//internal
#include "config.h"
#if defined(HAVE_MERO) || defined(HAVE_MOTR)
	#include "clovis_api.h"
#endif
#include "../common/Debug.hpp"
#include "Listings.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
Listings::Listings(void)
{
	std::srand(std::time(nullptr));
}

/*******************  FUNCTION  *********************/
Listings::~Listings(void)
{
	this->saveListings();
}

/*******************  FUNCTION  *********************/
ObjectId Listings::getObjectId(const std::string & listing, const std::string name)
{
	//check
	assume(listing.empty() == false, "Invalid empty listing filename !");
	assume(name.empty() == false, "Invalid ressource name !");

	//CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::mutex> lockGuard(this->mutex);

		//check if exist
		auto listingIt = this->listings.find(listing);
		if (listingIt == this->listings.end()) {
			return createListingAndObjectId(listing, name);
		} else {
			auto objIt = listingIt->second.find(name);
			if (objIt == listingIt->second.end()) {
				return createObjectId(listingIt->second, name);
			} else {
				return objIt->second;
			}
		}
	}
}

/*******************  FUNCTION  *********************/
ObjectId Listings::createListingAndObjectId(const std::string & listing, const std::string & name)
{
	FILE * fp = fopen(listing.c_str(), "r");
	if (fp != NULL) {
		ObjectMap & lst = this->loadListing(listing);
		auto it = lst.find(name);
		if (it != lst.end()) {
			return it->second;
		} else {
			return createObjectId(lst, name);
		}
	} else {
		return createObjectId(listings[listing], name);
	}
}

/*******************  FUNCTION  *********************/
ObjectId Listings::createObjectId(ObjectMap & listing, const std::string & name)
{
	//create
	#ifdef MERO_FOUND_WITH_GENREATE_ID
		ObjectId id;
		int res = c0appz_generate_id(&id.high, &id.low);
		assume(res == 0, "Failed to generate object ID !");
	#else
		ObjectId id = {
			((int64_t)std::rand() << 32) + (int64_t)std::rand(),
			((int64_t)std::rand() << 32) + (int64_t)std::rand()
		};
	#endif

	//register
	listing[name] = id;

	//ret
	return id;
}

/*******************  FUNCTION  *********************/
ObjectMap & Listings::loadListing(const std::string & listing)
{
	//open
	FILE * fp = fopen(listing.c_str(), "r");

	//does not exist
	if (fp == NULL)
		return this->listings[listing];

	//lst
	auto & store = this->listings[listing];

	//read line
	while(!feof(fp)) {
		char buffer[4096];
		char * buf = fgets(buffer, sizeof(buffer), fp);
		if (buf != NULL) {
			char objname[4096];
			ObjectId obj;
			int res = sscanf(buf, "%s\t%lx:%lx", objname, &obj.high, &obj.low);
			assumeArg(res == 3, "Invalid format in listing '%1' : %2 ")
				.arg(listing)
				.arg(buf)
				.end();
			store[objname] = obj;
		}
	}

	//close
	fclose(fp);

	//return
	return store;
}

/*******************  FUNCTION  *********************/
void Listings::saveListings(void)
{
	for(auto it : listings) {
		saveListing(it.second, it.first);
	}
}

/*******************  FUNCTION  *********************/
void Listings::saveListing(ObjectMap & listing, const std::string & name)
{
	//open
	FILE * fp = fopen(name.c_str(), "w+");

	//check
	assumeArg(fp != NULL, "Fail to open listing file '%1' : %2")
		.arg(name)
		.argStrErrno()
		.end();
	
	//save all
	for (auto it : listing) {
		fprintf(fp, "%s\t%lx:%lx\n", it.first.c_str(), it.second.high, it.second.low);
	}

	//close
	fclose(fp);
}
