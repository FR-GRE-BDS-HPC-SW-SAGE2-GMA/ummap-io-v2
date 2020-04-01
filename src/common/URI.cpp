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
const char * cst_uri_regexp = "([a-zA-Z0-9]+)://([a-zA-Z0-9_/.:-]+)([?]([a-zA-Z0-9]+=[a-zA-Z0-9_/+-]+)?(&[a-zA-Z0-9]+=[a-zA-Z0-9_/+-]+)*)?";

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
URI::URI(const std::string & uri)
{
	this->parse(uri);
}

/*******************  FUNCTION  *********************/
URI::~URI(void)
{

}

/*******************  FUNCTION  *********************/
void URI::reset(void)
{
	this->uri.clear();
	this->path.clear();
	this->type.clear();
	this->params.clear();
}

/*******************  FUNCTION  *********************/
void URI::parse(const std::string & uri)
{
	//clear
	this->reset();

	//build regex
	std::regex reg(cst_uri_regexp);
	std::smatch matches;

	if(std::regex_search(uri, matches, reg)) {
		this->uri = uri;
		this->type = matches[1];
		this->path = matches[2];
		if (matches[3] != "")
			for (int i = 4 ; i < matches.size() ; i++) {
				//split
				std::string opt = matches[i];
				size_t sep = opt.find('=');

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
const std::string & URI::getURI(void) const
{
	return this->uri;
}

/*******************  FUNCTION  *********************/
const std::string & URI::getType(void) const
{
	return this->type;
}

/*******************  FUNCTION  *********************/
const std::string & URI::getPath(void) const
{
	return this->path;
}

/*******************  FUNCTION  *********************/
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
