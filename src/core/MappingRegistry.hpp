/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_MAPPING_REGISTRY_HPP
#define UMMAP_MAPPING_REGISTRY_HPP

/********************  HEADERS  *********************/
//config
#include "config.h"
//std
#include <list>
//htopml
#ifdef HAVE_HTOPML
#include <htopml/JsonState.h>
#endif
//local
#include "portability/Spinlock.hpp"
#include "Mapping.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  STRUCT  *********************/
/**
 * Define an entry in the mapping registry.
**/
struct MappingRegistryEntry
{
	/** Pointer to the mapping. **/
	Mapping * mapping;
	/** Base address. **/
	char * base;
	/** End address **/
	char * end;
};

/*********************  CLASS  **********************/
/**
 * Class to implement a registry of all the active mappings.
**/
class MappingRegistry
{
	public:
		MappingRegistry(void);
		~MappingRegistry(void);
		void registerMapping(Mapping * mapping);
		void unregisterMapping(Mapping * mapping);
		void deleteAllMappings(void);
		bool isEmpty(void);
		Mapping * getMapping(void * addr);
	public:
		#ifdef HAVE_HTOPML
		friend void convertToJson(htopml::JsonState & json,const MappingRegistry & value);
		#endif
	private:
		bool contain(Mapping * mapping);
	private:
		/** Spinlock to protect the registry from concurrent accesses. **/
		Spinlock lock;
		/** List of entries **/
		std::list<MappingRegistryEntry> entries;
};

/*******************  FUNCTION  *********************/
#ifdef HAVE_HTOPML
void convertToJson(htopml::JsonState & json,const MappingRegistry & value);
#endif //HAVE_HTOPML

}

#endif //UMMAP_MAPPING_REGISTRY_HPP
