/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <cstdio>
#include <cassert>
#include "../common/Debug.hpp"
#include "../common/HumanUnits.hpp"
#include "../drivers/FDDriver.hpp"
#include "../drivers/MemoryDriver.hpp"
#include "../drivers/DummyDriver.hpp"
#include "../drivers/MmapDriver.hpp"
#include "../policies/FifoPolicy.hpp"
#include "MeroRessource.hpp"
#include "Uri.hpp"
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
	Uri parser(realUri);

	//cases
	std::string type = parser.getType();
	Driver * driver = NULL;
	if (type == "file") {
		driver = this->buildDriverFOpen(parser.getPath(), parser.getParam("mode", "w+"));
	} else if (type == "mem") {
		size_t memsize = fromHumanMemSize(parser.getPath());
		driver = new MemoryDriver(memsize);
	} else if (type == "dummy") {
		size_t value = atol(parser.getPath().c_str());
		driver = new DummyDriver(value);
	} else if (type == "mero" || type == "merofile" ) {
		driver = buildDriverMero(parser);
	} else if (type == "mmap" || type == "dax" ) {
		driver = this->buildDriverFOpenMmap(parser.getPath(), parser.getParam("mode", "w+"));
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to build driver : %1").arg(uri).end();
		return NULL;
	}

	//attach uri for debugging
	driver->setUri(uri);

	//return
	return driver;
}

/*******************  FUNCTION  *********************/
Policy * UriHandler::buildPolicy(const std::string & uri, bool local)
{
	//check
	assert(uri.empty() == false);

	//replace
	std::string realUri = this->replaceVariables(uri);

	//parse
	Uri parser(uri);

	//cases
	std::string type = parser.getType();
	Policy * policy = NULL;
	if (type == "fifo") {
		size_t memsize = fromHumanMemSize(parser.getPath());
		policy = new FifoPolicy(memsize, local);
	} else if (type == "none") {
		policy = NULL;
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to build policy : %1").arg(uri).end();
	}

	//set
	policy->setUri(uri);

	//ret
	return policy;
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

/*******************  FUNCTION  *********************/
Driver * UriHandler::buildDriverFOpenMmap(const std::string & fname, const std::string & mode)
{
	//open
	FILE * fp = fopen(fname.c_str(), mode.c_str());
	assumeArg(fp != NULL, "Fail to open file '%1': %2")
		.arg(fname)
		.argStrErrno()
		.end();
	
	//create driver
	Driver * res = new MmapDriver(fileno(fp));

	//close
	fclose(fp);

	//return
	return res;
}

/*******************  FUNCTION  *********************/
//mero://1234:1234
Driver * UriHandler::buildDriverMero(const Uri & uri)
{
	//check
	assert(uri.getType() == "mero" || uri.getType() == "merofile");

	//id
	ObjectId id;
	const std::string & path = uri.getPath();
	if (path == "auto") {
		const std::string & listing = uri.getParam("listing");
		const std::string & name = uri.getParam("name");
		id = this->objectIdListings.getObjectId(listing, name);
	} else if (sscanf(path.c_str(), "%lx:%lx", &id.low, &id.high) == 2) {
		//nothing more to do
	} else {
		UMMAP_FATAL_ARG("Invalid object ID in mero URI : %1")
			.arg(uri.getURI())
			.end();
		return NULL;
	}

	//init mero
	this->ressourceHandler.checkRessource<MeroRessource>("mero");

	//build driver
	#ifdef HAVE_MERO
		//TODO do mero real
	#else
		if (uri.getType() == "merofile")
		{
			//warn
			UMMAP_WARNING("Mero is not available, using fake fopen mode for tests");

			//replacement
			char fname[1024];
			sprintf(fname, "%lx:%lx", id.low, id.high);
			return buildDriverFOpen(fname, "w+");
		} else {
			UMMAP_FATAL_ARG("Mero is not available, cannot use uri : %1").arg(uri.getURI()).end();
			return NULL;
		}
	#endif
}
