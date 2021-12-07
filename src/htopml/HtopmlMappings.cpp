/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
#include <cassert>
#include "common/Debug.hpp"
#include "HtopmlMappings.hpp"
#include <htopml/TemplatePageHttpNode.h>

/*******************  NAMESPACE  ********************/
namespace ummapio
{

/********************  GLOBALS  *********************/
MappingPointer HtopmlMappingsHttpNode::registry = {NULL};

/*******************  FUNCTION  *********************/
HtopmlMappingsHttpNode::HtopmlMappingsHttpNode ( const std::string& addr )
	:htopml::JsonHttpNode<MappingPointer>(addr,&registry)
{

}

/*******************  FUNCTION  *********************/
void HtopmlMappingsHttpNode::registerMapping ( MappingRegistry* reg )
{
	HtopmlMappingsHttpNode::registry.registry = reg;
}

/*******************  FUNCTION  *********************/
void htopmlRegisterMappingsPage ( htopml::HtopmlHttpServer & server )
{
	
	//setup dynamic nodes
	server.registerHttpNode(new HtopmlMappingsHttpNode("/ummap-io/mappings.json"),true);
	
	//mount some static files
	server.quickRegisterFile("/ummap-io/mappings.css",HTOPML_PLUGIN_WWW "/mappings.css",false);
	server.quickRegisterFile("/ummap-io/mappings.js",HTOPML_PLUGIN_WWW "/mappings.js",false);
	server.registerHttpNode(new htopml::TemplatePageHttpNode("/ummap-io/mappings.html",HTOPML_PLUGIN_WWW "/mappings.html",false));
	
	//setup icon
	std::string icon = "/ummap-io/icons/ram.png";

	//register icon file 
	server.quickRegisterFile(icon, HTOPML_PLUGIN_WWW "/icons/ram.png",false);
	
	//add menu entry
	server.addMenuEntry("Ummap Mappings","/ummap-io/mappings.html",icon);
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState & json,const MappingPointer & value)
{
	convertToJson(json, *value.registry);
}

/*********************  CONSTS  *********************/
HTOPML_REGISTER_MODULE(htopmlRegisterMappingsPage);

}
