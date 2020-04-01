/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cassert>
#include <mutex>
//local
#include "common/Debug.hpp"
#include "PolicyRegistry.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap_io;

/*******************  FUNCTION  *********************/
PolicyRegistry::PolicyRegistry(void)
{

}

/*******************  FUNCTION  *********************/
PolicyRegistry::~PolicyRegistry(void)
{
	deleteAll();
}

/*******************  FUNCTION  *********************/
void PolicyRegistry::registerPolicy(const std::string & name, Policy * policy)
{
	//check
	assert(name.empty() == false);

	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//append
		assumeArg(this->entries.find(name) == this->entries.end(), 
			"A policy group already exist with this name : %1")
			.arg(name)
			.end();

		//put
		this->entries[name] = policy;
	}
}

/*******************  FUNCTION  *********************/
void PolicyRegistry::unregisterPolicy(const std::string & name)
{
	//check
	assert(name.empty() == false);

	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//delete
		auto it = this->entries.find(name);
		if (it != this->entries.end()) {
			delete it->second;
			this->entries.erase(it);
		}
	}
}

/*******************  FUNCTION  *********************/
Policy * PolicyRegistry::get(const std::string &name)
{
	//check
	assert(name.empty() == false);

	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//search
		auto it = this->entries.find(name);

		//return
		if (it != this->entries.end())
			return it->second;
		else
			return NULL;
	}
}

/*******************  FUNCTION  *********************/
void PolicyRegistry::deleteAll(void)
{
	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//loop to delete
		for (auto it : this->entries)
			delete it.second;

		//clear all
		this->entries.clear();
	}
}

/*******************  FUNCTION  *********************/
bool PolicyRegistry::isEmpty(void)
{
	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);
		return this->entries.empty();
	}
}
