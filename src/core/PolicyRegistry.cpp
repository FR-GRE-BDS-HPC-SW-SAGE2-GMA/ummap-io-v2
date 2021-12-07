/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cassert>
#include <mutex>
//local
#include "common/Debug.hpp"
#include "PolicyRegistry.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Constructor, currently do nothing.
**/
PolicyRegistry::PolicyRegistry(void)
{

}

/*******************  FUNCTION  *********************/
/**
 * Destructor of the policty registry. It destroy all the registered
 * policies.
**/
PolicyRegistry::~PolicyRegistry(void)
{
	deleteAll();
}

/*******************  FUNCTION  *********************/
/**
 * Register a new policy.
 * @param name Define the name attached to the policy.
 * @param policy Pointer to the policy to be registered under name.
**/
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
/**
 * Remove a policy from the registry and delete it.
 * @param name Name of the policy to remove.
**/
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
/**
 * Search the requested policy by its name.
 * @param name Name of the wanted policy.
 * @param nullNotFound If true return NULL on not found (for unit tests).
 * @return Return the requested policy by pointer or NULL if not found.
**/
Policy * PolicyRegistry::get(const std::string &name, bool nullNotFound)
{
	//check
	assert(name.empty() == false);

	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//search
		auto it = this->entries.find(name);

		//return
		if (it != this->entries.end()) {
			return it->second;
		} else {
			if (nullNotFound == false)
				UMMAP_FATAL_ARG("Failed to find policy in global registry with name '%1'").arg(name).end();
			return NULL;
		}
	}
}

/*******************  FUNCTION  *********************/
/**
 * Delete all the registred policies.
**/
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
/**
 * Check if there is at least one policy or if the registry is empty.
**/
bool PolicyRegistry::isEmpty(void)
{
	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);
		return this->entries.empty();
	}
}
