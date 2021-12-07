/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_HTOPML_MAPPINGS_HPP
#define UMMAP_HTOPML_MAPPINGS_HPP

/********************  HEADERS  *********************/
#include <htopml/JsonHttpNode.h>
#include <htopml/HtopmlHttpServer.h>
#include "../core/MappingRegistry.hpp"

/*******************  NAMESPACE  ********************/
namespace ummapio
{

/*********************  STRUCT  *********************/
struct MappingPointer
{
	MappingRegistry* registry;
};

/*********************  CLASS  **********************/
class HtopmlMappingsHttpNode : public htopml::JsonHttpNode<MappingPointer>
{
	public:
		HtopmlMappingsHttpNode(const std::string & addr);
		static void registerMapping(MappingRegistry * registry);
	private:
		static MappingPointer registry;
};

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState & json,const MappingPointer & value);

}

#endif //UMMAP_HTOPML_MAPPINGS_HPP
