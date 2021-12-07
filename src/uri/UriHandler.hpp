/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_URI_HANDLER_HPP
#define UMMAP_URI_HANDLER_HPP

/********************  HEADERS  *********************/
#include <string>
#include <map>
#include <mutex>
#include "../core/Driver.hpp"
#include "../core/Policy.hpp"
#include "../public-api/ummap.h"
#include "Uri.hpp"
#include "Listings.hpp"
#include "RessourceHandler.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
class UriHandler
{
	public:
		UriHandler(void);
		~UriHandler(void);
		void registerVariable(const std::string & name, const std::string & value);
		void registerVariable(const std::string & name, int value);
		void registerVariable(const std::string & name, size_t value);
		Driver * buildDriver(const std::string & uri);
		Policy * buildPolicy(const std::string & uri, bool local);
		int applyCow(void * addr, const std::string & uri, bool allowExist);
		int applySwitch(void * addr, const std::string & uri, ummap_switch_clean_t cleanAction);
		std::string replaceVariables(std::string value);
		void initMero(const std::string & ressourceFile, int ressourceIndex);
	private:
		Driver * buildDriverFOpen(const std::string & fname, const std::string & mode);
		Driver * buildDriverFOpenMmap(const std::string & fname, const std::string & mode);
		Driver * buildDriverMero(const Uri & uri);
		Driver * buildDriverIoc(const Uri & uri);
		ObjectId getIocObjectId(const Uri & uri);
	private:
		std::mutex variablesMutes;
		std::map<std::string, std::string> variables;
		Listings objectIdListings;
		RessourceHandler ressourceHandler;
};

}

#endif //UMMAP_URI_HANDLER_HPP
