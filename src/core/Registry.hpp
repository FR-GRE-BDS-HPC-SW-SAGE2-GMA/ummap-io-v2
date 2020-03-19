/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_REGISTRY_HPP
#define UMMAP_REGISTRY_HPP

/********************  HEADERS  *********************/
//std
#include <list>
//local
#include "portability/Spinlock.hpp"
#include "Mapping.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  STRUCT  *********************/
struct RegistryEntry
{
	Mapping * mapping;
	char * base;
	char * end;
};

/*********************  CLASS  **********************/
class Registry
{
	public:
		Registry(void);
		~Registry(void);
		void registerMapping(Mapping * mapping);
		void unregisterMapping(Mapping * mapping);
		Mapping * getMapping(void * addr);
	private:
		bool contain(Mapping * mapping);
	private:
		Spinlock lock;
		std::list<RegistryEntry> entries;
};

}

#endif //UMMAP_REGISTRY_HPP