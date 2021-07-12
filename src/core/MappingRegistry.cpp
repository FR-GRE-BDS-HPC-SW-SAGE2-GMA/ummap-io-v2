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
#include "MappingRegistry.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Constructor of the mapping registry. It currently does nothing.
**/
MappingRegistry::MappingRegistry(void)
{

}

/*******************  FUNCTION  *********************/
/**
 * Destructor of the mapping registry. It by default destroy all the registered
 * mapping before returning.
**/
MappingRegistry::~MappingRegistry(void)
{
	deleteAllMappings();
}

/*******************  FUNCTION  *********************/
/**
 * Registry the given mapping to the registry. This functions
 * if protected by a spinlock for thread safetry.
 * @param mapping Pointer to the mapping to register.
**/
void MappingRegistry::registerMapping(Mapping * mapping)
{
	//check
	assert(mapping != NULL);
	assert(!contain(mapping));

	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//extract
		char * base = (char*)mapping->getAddress();
		size_t size = mapping->getSize();

		//build
		MappingRegistryEntry entry = {
			mapping,
			base,
			base + size,
		};

		//register
		this->entries.push_back(entry);
	}
}

/*******************  FUNCTION  *********************/
/**
 * Check if the registry alredy contain the given mapping.
 * @param mapping Pointer to the mapping to check.
**/
bool MappingRegistry::contain(Mapping * mapping)
{
	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//check
		assert(mapping != NULL);

		//loop all
		for (auto it : this->entries)
			if (it.mapping == mapping)
				return true;
		
		//not found
		return false;
	}
}

/*******************  FUNCTION  *********************/
/**
 * De-register the given mapping.
 * @param mapping Pointer to the mapping to de-register.
**/
void MappingRegistry::unregisterMapping(Mapping * mapping)
{
	//check
	assert(mapping != NULL);

	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//loop
		for (auto it = entries.begin() ; it != entries.end() ; ++it) {
			if (it->mapping == mapping) {
				auto tmp = it;
				++it;
				entries.erase(tmp);
			}
		}
	}
}

/*******************  FUNCTION  *********************/
/**
 * Find the mapping from the given address.
 * @param addr An address which should be contained by one of the registered mapping.
 * @return Return the address of the related mapping of NULL if not found.
**/
Mapping * MappingRegistry::getMapping(void * addr)
{
	//check
	assert(addr != NULL);

	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//loop
		for (auto it : this->entries)
			if (addr >= it.base && addr < it.end)
				return it.mapping;
		
		//not found
		return NULL;
	}
}

/*******************  FUNCTION  *********************/
/**
 * Delete all the mappings registed by the registry.
**/
void MappingRegistry::deleteAllMappings(void)
{
	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);

		//loop to delete
		for (auto it : this->entries)
			delete it.mapping;

		//clear all
		this->entries.clear();
	}
}

/*******************  FUNCTION  *********************/
/**
 * Check if the registry is empty.
**/
bool MappingRegistry::isEmpty(void)
{
	//CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->lock);
		return this->entries.empty();
	}
}

/*******************  FUNCTION  *********************/
#ifdef HAVE_HTOPML
/**
 * When habing htopml, convert the registry to json format.
**/
void ummapio::convertToJson(htopml::JsonState & json,const MappingRegistry & value)
{
	json.openArray();
	for (auto it : value.entries)
		json.printValue(*it.mapping);
	json.closeArray();
}
#endif //HAVE_HTOPML
