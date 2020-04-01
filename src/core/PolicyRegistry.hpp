/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
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
namespace ummap_io
{

/*********************  CLASS  **********************/
class PolicyRegistry
{
	public:
		PolicyRegistry(void);
		~PolicyRegistry(void);
		void registerPolicy(const std::string & name, Policy * policy);
		void unregisterPolicy(const std::string & name);
		void deleteAll(void);
		bool isEmpty(void);
		Policy * get(const std::string & name);
	private:
		Spinlock lock;
		std::map<std::string, Policy *> entries;
};

}

#endif //UMMAP_POLICY_REGISTRY_HPP