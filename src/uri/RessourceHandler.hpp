/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_RESSOURCE_HANDLER_HPP
#define UMMAP_RESSOURCE_HANDLER_HPP

/********************  HEADERS  *********************/
#include <string>
#include <map>
#include <mutex>

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
class Ressource
{
	public:
		Ressource(void) {};
		virtual ~Ressource(void) {};
};

/*********************  CLASS  **********************/
class RessourceHandler
{
	public:
		RessourceHandler(void);
		~RessourceHandler(void);
		template <class T> void checkRessource(const std::string & name);
	private:
		std::mutex mutex;
		std::map<std::string, Ressource *> ressourceMap;
};

/*******************  FUNCTION  *********************/
template <class T> 
void RessourceHandler::checkRessource(const std::string & name)
{
	//CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::mutex> lockGuard(this->mutex);

		//search
		auto it = this->ressourceMap.find(name);

		//create
		if (it == this->ressourceMap.end()) {
			this->ressourceMap[name] = new T;
		}
	}
}

}

#endif //UMMAP_RESSOURCE_HANDLER_HPP
