/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_URI_HPP
#define UMMAP_URI_HPP

/********************  HEADERS  *********************/
#include <string>
#include <map>

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * Unified Ressource Identifer to build object with a simple semantic.
**/
class Uri
{
	public:
		Uri(const std::string & uri);
		~Uri(void);
		const std::string & getURI(void) const;
		const std::string & getType(void) const;
		const std::string & getPath(void) const;
		const std::string getParam(const std::string & name) const;
		const std::string getParam(const std::string & name, const std::string & defaultValue) const;
		int getParamAsInt(const std::string & name) const;
		int getParamAsInt(const std::string & name, const int defaultValue) const;
		size_t getParamAsSizet(const std::string & name) const;
		size_t getParamAsSizet(const std::string & name, const size_t defaultValue) const;
	private:
		void noRegexParse(const std::string & uri);
		void regexParse(const std::string & uri);
		void parse(const std::string & uri);
		void reset(void);
	private:
		/** Full address **/
		std::string uri;
		/** Type of object to build (eg: file://path will have type 'file') **/
		std::string type;
		/** Path part of the URI (eg! file://path will have path saying 'path') **/
		std::string path;
		/** Map of parameters (eg: file://path?param1=value1&param2=value2) **/
		std::map<std::string, std::string> params;
};

}

#endif //UMMAP_URI_HPP
