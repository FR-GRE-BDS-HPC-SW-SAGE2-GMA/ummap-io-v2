/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <regex>
#include <iostream>
#include "../common/Debug.hpp"
#include "Uri.hpp"

/********************  CONSTS  **********************/
/**
 * Regular expression to check correctness and extract parts.
**/
const char * cst_uri_regexp = "^([a-zA-Z0-9]+)://([a-zA-Z0-9_/.:-]+)([?]([a-zA-Z0-9]+=[a-zA-Z0-9._/+-]+)?(&[a-zA-Z0-9]+=[a-zA-Z0-9._/+-]+)*)?$";

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * URI constructor, it mostly directly parse the given URI.
 * 
 * It can exit in case of failure.
 * 
 * @param uri URI to setup.
**/
Uri::Uri(const std::string & uri)
{
	this->parse(uri);
}

/*******************  FUNCTION  *********************/
/**
 * Destructor, do nothing for now.
**/
Uri::~Uri(void)
{

}

/*******************  FUNCTION  *********************/
/**
 * Reset the content in case of next call to parse().
**/
void Uri::reset(void)
{
	this->uri.clear();
	this->path.clear();
	this->type.clear();
	this->params.clear();
}

/*******************  FUNCTION  *********************/
/**
 * Parse the given string URI and extract parts for latter use.
 * 
 * It cas exit in case of failure.
 * 
 * @param uri The URI string to parse.
**/
void Uri::parse(const std::string & uri)
{
	//clear
	this->reset();

	//build regex
	std::regex reg(cst_uri_regexp);
	std::smatch matches;

	//apply regexp
	if(std::regex_match(uri, matches, reg)) {
		this->uri = uri;
		this->type = matches[1];
		this->path = matches[2];
		//has params
		if (matches[3] != "")
		{
			//vars
			std::string buf;
			std::string name;
			std::string args = matches[3];
			
			//for last loop
			args += '&';

			//loop
			for (int i = 1 ; i < args.size() ; i++) {
				if (args[i] == '&') {
					//insert
					this->params[name] = buf;

					//debug
					//std::cout << name << "=" << buf << std::endl;

					//reset
					buf.clear();
					name.clear();
				} else if (args[i] == '=') {
					name = buf;
					buf.clear();
				} else {
					buf += args[i];
				}
			}
		}
	} else {
		UMMAP_FATAL_ARG("Unrecongnized ummap URI format : %1").arg(uri).end();
	}
}

/*******************  FUNCTION  *********************/
/**
 * @return Return the full URI as it has been passed to parse() or constructor.
**/
const std::string & Uri::getURI(void) const
{
	return this->uri;
}

/*******************  FUNCTION  *********************/
/**
 * Return type part of the URI.
 * 
 * Eg: 'file://path' will return 'path'.
 * 
 * @return The type as a string.
**/
const std::string & Uri::getType(void) const
{
	return this->type;
}

/*******************  FUNCTION  *********************/
/**
 * Retrurn the path part of the URI
 * 
 * Eg: 'file:///dir/file' will return '/dir/file'
 * 
 * @return The path as a string.
**/
const std::string & Uri::getPath(void) const
{
	return this->path;
}

/*******************  FUNCTION  *********************/
/**
 * Return the parameter value extracted form the URI.
 * 
 * Eg: 'file://path?param=value' will return 'value' for parameter 'param'
 * 
 * Caution, this function abort if the given parameter is not found.
 * 
 * @param name Name of the parameter to extract
 * @return Return the value of the requested parameter.
**/
const std::string Uri::getParam(const std::string & name) const
{
	//search
	auto it = this->params.find(name);

	//check & apply
	if (it != this->params.end()) {
		return it->second;
	} else {
		UMMAP_FATAL_ARG("Fail to find required parameter '%1' in uri: %2").arg(name).arg(uri).end();
		return "";
	}
}

/*******************  FUNCTION  *********************/
/**
 * Return the parameter value extracted form the URI. This variant return
 * a default value if the requested parameter is not found.
 * 
 * Eg: 'file://path?param=value' will return 'value' for parameter 'param'
 * 
 * @param name Name of the parameter to extract
 * @param default Default value to return if not found.
 * @return Return the value of the given parameter or the default value if not found.
**/
const std::string Uri::getParam(const std::string & name, const std::string & defaultValue) const
{
	//search
	auto it = this->params.find(name);

	//check & apply
	if (it != this->params.end())
		return it->second;
	else
		return defaultValue;
}

/*******************  FUNCTION  *********************/
/**
 * Return the parameter value extracted form the URI as an integer.
 * 
 * Eg: 'file://path?param=value' will return 'value' for parameter 'param'
 * 
 * Caution, this function abort if the given parameter is not found.
 * 
 * @param name Name of the parameter to extract
 * @return Return the value of the requested parameter.
**/
int Uri::getParamAsInt(const std::string & name) const
{
	//search
	auto it = this->params.find(name);

	//check & apply
	if (it != this->params.end()) {
		return atoi(it->second.c_str());
	} else {
		UMMAP_FATAL_ARG("Fail to find required parameter '%1' in uri: %2").arg(name).arg(uri).end();
		return 0;
	}
}

/*******************  FUNCTION  *********************/
/**
 * Return the parameter value extracted form the URI as an integer. This variant return
 * a default value if the requested parameter is not found.
 * 
 * Eg: 'file://path?param=value' will return 'value' for parameter 'param'
 * 
 * @param name Name of the parameter to extract
 * @param default Default value to return if not found.
 * @return Return the value of the given parameter or the default value if not found.
**/
int Uri::getParamAsInt(const std::string & name, int defaultValue) const
{
	//search
	auto it = this->params.find(name);

	//check & apply
	if (it != this->params.end())
		return atoi(it->second.c_str());
	else
		return defaultValue;
}

/*******************  FUNCTION  *********************/
/**
 * Return the parameter value extracted form the URI as a size_t.
 * 
 * Eg: 'file://path?param=value' will return 'value' for parameter 'param'
 * 
 * Caution, this function abort if the given parameter is not found.
 * 
 * @param name Name of the parameter to extract
 * @return Return the value of the requested parameter.
**/
size_t Uri::getParamAsSizet(const std::string & name) const
{
	//search
	auto it = this->params.find(name);

	//check & apply
	if (it != this->params.end()) {
		return atol(it->second.c_str());
	} else {
		UMMAP_FATAL_ARG("Fail to find required parameter '%1' in uri: %2").arg(name).arg(uri).end();
		return 0;
	}
}

/*******************  FUNCTION  *********************/
/**
 * Return the parameter value extracted form the URI as an size_t. This variant return
 * a default value if the requested parameter is not found.
 * 
 * Eg: 'file://path?param=value' will return 'value' for parameter 'param'
 * 
 * @param name Name of the parameter to extract
 * @param default Default value to return if not found.
 * @return Return the value of the given parameter or the default value if not found.
**/
size_t Uri::getParamAsSizet(const std::string & name, size_t defaultValue) const
{
	//search
	auto it = this->params.find(name);

	//check & apply
	if (it != this->params.end())
		return atol(it->second.c_str());
	else
		return defaultValue;
}
