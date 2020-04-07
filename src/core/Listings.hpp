/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_LISTINGS_HPP
#define UMMAP_LISTINGS_HPP

/********************  HEADERS  *********************/
//std
#include <mutex>
#include <string>
#include <map>
#include <stdint.h>

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  STRUCT  *********************/
struct ObjectId
{
	uint64_t low;
	uint64_t high;
};

/*********************  TYPES  **********************/
typedef std::map<std::string, ObjectId> ObjectMap;
typedef std::map<std::string, ObjectMap> ListingMap;

/*********************  CLASS  **********************/
class Listings
{
	public:
		Listings(void);
		~Listings(void);
		ObjectId getObjectId(const std::string & listing, const std::string name);
	private:
		ObjectMap & loadListing(const std::string & listing);
		void saveListings(void);
		void saveListing(ObjectMap & listing, const std::string & name);
		ObjectId createListingAndObjectId(const std::string & listing, const std::string & name);
		ObjectId createObjectId(ObjectMap & listing, const std::string & name);
	private:
		std::mutex mutex;
		ListingMap listings;
};

}

#endif //UMMAP_LISTINGS_HPP
