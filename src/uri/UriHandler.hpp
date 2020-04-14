/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_URI_HANDLER_HPP
#define UMMAP_URI_HANDLER_HPP

/********************  HEADERS  *********************/
#include <string>
#include <map>
#include <mutex>
#include "../core/Driver.hpp"
#include "../core/Policy.hpp"
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
		std::string replaceVariables(std::string value);
	private:
		Driver * buildDriverFOpen(const std::string & fname, const std::string & mode);
		Driver * buildDriverMero(const Uri & uri);
	private:
		std::mutex variablesMutes;
		std::map<std::string, std::string> variables;
		Listings objectIdListings;
		RessourceHandler ressourceHandler;
};

}

#endif //UMMAP_URI_HANDLER_HPP
