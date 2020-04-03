/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <regex>
#include <iostream>
#include "Debug.hpp"
#include "URI.hpp"

/********************  CONSTS  **********************/
/**
 * Regular expression to check correctness and extract parts.
**/
const char * cst_uri_regexp = "([a-zA-Z0-9]+)://([a-zA-Z0-9_/.:-]+)([?]([a-zA-Z0-9]+=[a-zA-Z0-9_/+-]+)?(&[a-zA-Z0-9]+=[a-zA-Z0-9_/+-]+)*)?";

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
URI::URI(const std::string & uri)
{
	this->parse(uri);
}

/*******************  FUNCTION  *********************/
/**
 * Destructor, do nothing for now.
**/
URI::~URI(void)
{

}

/*******************  FUNCTION  *********************/
/**
 * Reset the content in case of next call to parse().
**/
void URI::reset(void)
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
void URI::parse(const std::string & uri)
{
	//clear
	this->reset();

	//build regex
	std::regex reg(cst_uri_regexp);
	std::smatch matches;

	//apply regexp
	if(std::regex_search(uri, matches, reg)) {
		this->uri = uri;
		this->type = matches[1];
		this->path = matches[2];
		//has params
		if (matches[3] != "")
			//loop on params
			for (size_t i = 4 ; i < matches.size() ; i++) {
				//split param=value
				std::string opt = matches[i];
				size_t sep = opt.find('=');
				//has param=value
				if (sep != std::string::npos)
				{
					//extract
					std::string name;
					if (opt[0] == '&' || opt[0] == '?')
						name = opt.substr(1,sep-1);
					else
						name = opt.substr(0,sep);

					//extract
					std::string value = opt.substr(sep+1, std::string::npos);

					//insert
					this->params[name] = value;

					//debug
					//std::cout << "param[" << i << "] = " << matches[i] << std::endl;
					//std::cout << name << "=" << value << std::endl;
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
const std::string & URI::getURI(void) const
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
const std::string & URI::getType(void) const
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
const std::string & URI::getPath(void) const
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
const std::string URI::getParam(const std::string & name) const
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
const std::string URI::getParam(const std::string & name, const std::string & defaultValue) const
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
int URI::getParamAsInt(const std::string & name) const
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
int URI::getParamAsInt(const std::string & name, int defaultValue) const
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
size_t URI::getParamAsSizet(const std::string & name) const
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
size_t URI::getParamAsSizet(const std::string & name, size_t defaultValue) const
{
	//search
	auto it = this->params.find(name);

	//check & apply
	if (it != this->params.end())
		return atol(it->second.c_str());
	else
		return defaultValue;
}
