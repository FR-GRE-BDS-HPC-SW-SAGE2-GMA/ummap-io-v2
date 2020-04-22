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
MappingRegistry::MappingRegistry(void)
{

}

/*******************  FUNCTION  *********************/
MappingRegistry::~MappingRegistry(void)
{
	deleteAllMappings();
}

/*******************  FUNCTION  *********************/
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
		RegistryEntry entry = {
			.mapping = mapping,
			.base = base,
			.end = base + size,
		};

		//register
		this->entries.push_back(entry);
	}
}

/*******************  FUNCTION  *********************/
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
void ummapio::convertToJson(htopml::JsonState & json,const MappingRegistry & value)
{
	json.openArray();
	for (auto it : value.entries)
		json.printValue(*it.mapping);
	json.closeArray();
}
#endif //HAVE_HTOPML
