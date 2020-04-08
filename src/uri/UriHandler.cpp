/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <cstdio>
#include <cassert>
#include "../common/Debug.hpp"
#include "../drivers/FDDriver.hpp"
#include "../drivers/MemoryDriver.hpp"
#include "../drivers/DummyDriver.hpp"
#include "../policies/FifoPolicy.hpp"
#include "URI.hpp"
#include "UriHandler.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
UriHandler::UriHandler(void)
{
}

/*******************  FUNCTION  *********************/
UriHandler::~UriHandler(void)
{

}

/*******************  FUNCTION  *********************/
void UriHandler::registerVariable(const std::string & name, const std::string & value)
{
	this->variables[name] = value;
}

/*******************  FUNCTION  *********************/
void UriHandler::registerVariable(const std::string & name, int value)
{
	char buffer[256];
	sprintf(buffer, "%d", value);
	this->variables[name] = buffer;
}

/*******************  FUNCTION  *********************/
void UriHandler::registerVariable(const std::string & name, size_t value)
{
	char buffer[256];
	sprintf(buffer, "%zu", value);

	//CRITICAL SECTION
	{
		this->variablesMutes.lock();
		this->variables[name] = buffer;
		this->variablesMutes.unlock();
	}
}

/*******************  FUNCTION  *********************/
Driver * UriHandler::buildDriver(const std::string & uri)
{
	//check
	assert(uri.empty() == false);

	//replace
	std::string realUri = this->replaceVariables(uri);

	//parse
	URI parser(uri);

	//cases
	std::string type = parser.getType();
	if (type == "file") {
		return this->buildDriverFOpen(parser.getPath(), parser.getParam("mode", "w+"));
	} else if (type == "mem") {
		size_t memsize = atol(parser.getPath().c_str());
		return new MemoryDriver(memsize);
	} else if (type == "dummy") {
		size_t value = atol(parser.getPath().c_str());
		return new DummyDriver(value);
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to build driver : %1").arg(uri).end();
		return NULL;
	}
}

/*******************  FUNCTION  *********************/
Policy * UriHandler::buildPolicy(const std::string & uri, bool local)
{
	//check
	assert(uri.empty() == false);

	//replace
	std::string realUri = this->replaceVariables(uri);

	//parse
	URI parser(uri);

	//cases
	std::string type = parser.getType();
	if (type == "fifo") {
		size_t memsize = atol(parser.getPath().c_str());
		return new FifoPolicy(memsize, local);
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to build policy : %1").arg(uri).end();
		return NULL;
	}
}

/*******************  FUNCTION  *********************/
std::string UriHandler::replaceVariables(std::string value)
{
	//CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::mutex> lockGuard(this->variablesMutes);

		//replace
		for (auto it : this->variables) {
			const std::string var = std::string("{") + it.first + "}";
			const std::string & val = it.second;
			for (size_t index = value.find(var, 0); index != std::string::npos && var.length(); index = value.find(var, index + val.length() ) )
				value.replace(index, var.length(), val);
		}
	}

	//end
	return value;
}

/*******************  FUNCTION  *********************/
Driver * UriHandler::buildDriverFOpen(const std::string & fname, const std::string & mode)
{
	//open
	FILE * fp = fopen(fname.c_str(), mode.c_str());
	assumeArg(fp != NULL, "Fail to open file '%1': %2")
		.arg(fname)
		.argStrErrno()
		.end();
	
	//create driver
	Driver * res = new FDDriver(fileno(fp));

	//close
	fclose(fp);

	//return
	return res;
}
