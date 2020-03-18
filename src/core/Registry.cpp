/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cassert>
//local
#include "Registry.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
Registry::Registry(void)
{

}

/*******************  FUNCTION  *********************/
Registry::~Registry(void)
{

}

/*******************  FUNCTION  *********************/
void Registry::registerMapping(Mapping * mapping)
{
	//check
	assert(mapping != NULL);
	assert(!contain(mapping));

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

/*******************  FUNCTION  *********************/
bool Registry::contain(Mapping * mapping)
{
	//check
	assert(mapping != NULL);

	//loop all
	for (auto it : this->entries)
		if (it.mapping == mapping)
			return true;
	
	//not found
	return false;
}

/*******************  FUNCTION  *********************/
void Registry::unregisterMapping(Mapping * mapping)
{
	//check
	assert(mapping != NULL);

	//loop
	for (auto it = entries.begin() ; it != entries.end() ; ++it) {
		if (it->mapping == mapping) {
			auto tmp = it;
			++it;
			entries.erase(tmp);
		}
	}
}

/*******************  FUNCTION  *********************/
Mapping * Registry::getMapping(void * addr)
{
	//check
	assert(addr != NULL);

	//loop
	for (auto it : this->entries)
		if (addr >= it.base && addr < it.end)
			return it.mapping;
	
	//not found
	return NULL;
}