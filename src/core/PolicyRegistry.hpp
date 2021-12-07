/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_POLICY_REGISTRY_HPP
#define UMMAP_POLICY_REGISTRY_HPP

/********************  HEADERS  *********************/
//std
#include <map>
#include <string>
//local
#include "portability/Spinlock.hpp"
#include "Policy.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * The registry associate each registered policy to a name so we can
 * use this name to find it without keeping track of global pointers
 * over the application using ummap.
**/
class PolicyRegistry
{
	public:
		PolicyRegistry(void);
		~PolicyRegistry(void);
		void registerPolicy(const std::string & name, Policy * policy);
		void unregisterPolicy(const std::string & name);
		void deleteAll(void);
		bool isEmpty(void);
		Policy * get(const std::string & name, bool nullNotFound = false);
	private:
		/** Spinlock to protect the access to the map **/
		Spinlock lock;
		/** Map of policies indexed by name. **/
		std::map<std::string, Policy *> entries;
};

}

#endif //UMMAP_POLICY_REGISTRY_HPP
