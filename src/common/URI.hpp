/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
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
class URI
{
	public:
		URI(const std::string & uri);
		~URI(void);
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
		void parse(const std::string & uri);
		void reset(void);
	private:
		std::string uri;
		std::string type;
		std::string path;
		std::map<std::string, std::string> params;
};

}

#endif //UMMAP_URI_HPP
