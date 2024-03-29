/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstdio>
#include <cassert>
//unix
 #include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//internal
#include "config.h"
#include "../common/Debug.hpp"
#include "../common/HumanUnits.hpp"
#include "../drivers/FDDriver.hpp"
#include "../drivers/MemoryDriver.hpp"
#include "../drivers/DummyDriver.hpp"
#include "../drivers/MmapDriver.hpp"
#if defined(HAVE_MERO) || defined(HAVE_MOTR)
	#include "../drivers/ClovisDriver.hpp"
#endif
#ifdef HAVE_IOC_CLIENT
#include "../drivers/IocDriver.hpp"
#endif
#include "../policies/FifoPolicy.hpp"
#include "../policies/FifoWindowPolicy.hpp"
#include "../policies/LifoPolicy.hpp"
#include "../public-api/ummap.h"
#include "MeroRessource.hpp"
#include "IocRessource.hpp"
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
/**
 * Apply the copy on write operation on the given address for the given new URI.
 * @param addr An adress in the mapping to impact.
 * @param uri The new URI to apply on the driver.
 * @param allow_exist Allow to COW on an object which already exist.
 * @return 0 on success, negative value on error.
**/
int UriHandler::applyCow(void * addr, const std::string & uri, bool allowExist)
{
	//check
	assert(uri.empty() == false);

	//replace
	std::string realUri = this->replaceVariables(uri);

	//parse
	Uri parser(realUri);

	//cases
	std::string type = parser.getType();

	//apply
	if (type == "meroioc" || type == "clovisioc" || type == "ioc" || type == "iocfile" ) {
		ObjectId id = getIocObjectId(parser);
		return ummap_cow_ioc(addr, id.high, id.low, allowExist);
	} else if (type == "mero" || type == "clovis" || type == "merofile" || type == "motr") {
		ObjectId id = getIocObjectId(parser);
		return ummap_cow_clovis(addr, id.high, id.low, allowExist);
	} else if (type == "dummy") {
		//we do nothing
		return 0;
	} else if (type == "mem") {
		//we do nothing
		return 0;
	} else if (type == "mapanon") {
		//we do nothing
		return 0;
	} else if (type == "file") {
		return ummap_cow_fopen(addr, parser.getPath().c_str(), parser.getParam("mode", "").c_str(), allowExist);
	} else if (type == "mmap" || type == "dax") {
		return ummap_cow_dax_fopen(addr, parser.getPath().c_str(), parser.getParam("mode", "").c_str(), allowExist);
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to run COW operation : %1").arg(uri).end();
		return -1;
	}
}

/*******************  FUNCTION  *********************/
int UriHandler::applySwitch(void * addr, const std::string & uri, ummap_switch_clean_t cleanAction)
{
		//check
	assert(uri.empty() == false);

	//replace
	std::string realUri = this->replaceVariables(uri);

	//parse
	Uri parser(realUri);

	//cases
	std::string type = parser.getType();

	//apply
	if (type == "meroioc" || type == "clovisioc" || type == "ioc" || type == "iocfile" ) {
		ObjectId id = getIocObjectId(parser);
		return ummap_switch_ioc(addr, id.high, id.low, cleanAction);
	} else if (type == "mero" || type == "clovis" || type == "merofile" || type == "motr") {
		ObjectId id = getIocObjectId(parser);
		return ummap_switch_clovis(addr, id.high, id.low, cleanAction);
	} else if (type == "dummy") {
		//we do nothing
		return 0;
	} else if (type == "mem") {
		//we do nothing
		return 0;
	} else if (type == "mapanon") {
		//we do nothing
		return 0;
	} else if (type == "file") {
		return ummap_switch_fopen(addr, parser.getPath().c_str(), parser.getParam("mode", "").c_str(), cleanAction);
	} else if (type == "mmap" || type == "dax") {
		return ummap_switch_dax_fopen(addr, parser.getPath().c_str(), parser.getParam("mode", "").c_str(), cleanAction);
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to run switch operation : %1").arg(uri).end();
		return -1;
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
		driver = this->buildDriverFOpen(parser.getPath(), parser.getParam("mode", ""));
	} else if (type == "mem") {
		size_t memsize = fromHumanMemSize(parser.getPath());
		driver = new MemoryDriver(memsize);
	} else if (type == "dummy") {
		size_t value = atol(parser.getPath().c_str());
		driver = new DummyDriver(value);
	} else if (type == "mero" || type == "clovis" || type == "merofile" || type == "motr" ) {
		driver = buildDriverMero(parser);
	} else if (type == "meroioc" || type == "clovisioc" || type == "ioc" || type == "iocfile" ) {
		driver = buildDriverIoc(parser);
	} else if (type == "mmap" || type == "dax" ) {
		driver = this->buildDriverFOpenMmap(parser.getPath(), parser.getParam("mode", ""));
	} else if (type == "mmapanon") {
		driver = new MmapDriver(0, true);
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to build driver : %1").arg(uri).end();
		return NULL;
	}

	//attach uri for debugging
	driver->setUri(realUri);

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
	Uri parser(realUri);

	//cases
	std::string type = parser.getType();
	Policy * policy = NULL;
	if (type == "fifo") {
		size_t memsize = fromHumanMemSize(parser.getPath());
		assumeArg(memsize % 4096 == 0, "The given size is not multiple of the page size: %1 in %2").arg(parser.getPath()).arg(uri).end();
		policy = new FifoPolicy(memsize, local);
	} else if (type == "lifo") {
		size_t memsize = fromHumanMemSize(parser.getPath());
		assumeArg(memsize % 4096 == 0, "The given size is not multiple of the page size: %1 in %2").arg(parser.getPath()).arg(uri).end();
		policy = new LifoPolicy(memsize, local);
	} else if (type == "fifo-window") {
		size_t memSize = fromHumanMemSize(parser.getPath());
		size_t slidingSize = fromHumanMemSize(parser.getParam("window"));
		assumeArg(memSize % 4096 == 0, "The given size is not multiple of the page size: %1 in %2").arg(parser.getPath()).arg(uri).end();
		assumeArg(slidingSize % 4096 == 0, "The given size is not multiple of the page size: %1 in %2").arg(parser.getParam("window")).arg(uri).end();
		policy = new FifoWindowPolicy(memSize, slidingSize, local);
	} else if (type == "none") {
		policy = NULL;
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to build policy : %1").arg(uri).end();
	}

	//set
	if (policy != NULL)
		policy->setUri(realUri);

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
	//vars
	FILE * fp = NULL;
	int fd = -1;

	//if default value we use direct open and o_create
	if (mode == "") {
		fd = open(fname.c_str(), O_RDWR | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		assumeArg(fd > 0, "Fail to open file '%1': %2").arg(fname).argStrErrno().end();
	} else {
		//open
		fp = fopen(fname.c_str(), mode.c_str());
		assumeArg(fp != NULL, "Fail to open file '%1': %2").arg(fname).argStrErrno().end();
		fd = fileno(fp);
	}
	
	//create driver
	Driver * res = new FDDriver(fd);

	//close
	if (fp != NULL)
		fclose(fp);
	else
		close(fd);

	//return
	return res;
}

/*******************  FUNCTION  *********************/
Driver * UriHandler::buildDriverFOpenMmap(const std::string & fname, const std::string & mode)
{
	//vars
	FILE * fp = NULL;
	int fd = -1;

	//if default value we use direct open and o_create
	if (mode == "") {
		fd = open(fname.c_str(), O_RDWR | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		assumeArg(fd > 0, "Fail to open file '%1': %2").arg(fname).argStrErrno().end();
	} else {
		//open
		fp = fopen(fname.c_str(), mode.c_str());
		assumeArg(fp != NULL, "Fail to open file '%1': %2").arg(fname).argStrErrno().end();
		fd = fileno(fp);
	}
	
	//create driver
	Driver * res = new MmapDriver(fd);

	//close
	if (fp != NULL)
		fclose(fp);
	else
		close(fd);

	//return
	return res;
}

/*******************  FUNCTION  *********************/
//mero://1234:1234
Driver * UriHandler::buildDriverMero(const Uri & uri)
{
	//check
	assert(uri.getType() == "mero" || uri.getType() == "clovis" || uri.getType() == "merofile" || uri.getType() == "motr");

	//id
	ObjectId id;
	const std::string & path = uri.getPath();
	if (path == "auto") {
		const std::string & listing = uri.getParam("listing");
		const std::string & name = uri.getParam("name");
		id = this->objectIdListings.getObjectId(listing, name);
	} else if (sscanf(path.c_str(), "%ld:%ld", &id.high, &id.low) == 2) {
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
	#if defined(HAVE_MERO) || defined(HAVE_MOTR)
		m0_uint128 m0id = {.u_hi = id.high, .u_lo = id.low};
		return new ClovisDriver(m0id, true);
	#else
		if (uri.getType() == "merofile")
		{
			//warn
			UMMAP_WARNING("Mero is not available, using fake fopen mode for tests");

			//replacement
			char fname[1024];
			sprintf(fname, "%lx:%lx", id.high, id.low);
			return buildDriverFOpen(fname, "");
		} else {
			UMMAP_FATAL_ARG("Mero is not available, cannot use uri : %1").arg(uri.getURI()).end();
			return NULL;
		}
	#endif
}

/*******************  FUNCTION  *********************/
ObjectId UriHandler::getIocObjectId(const Uri & uri)
{
	//vars
	ObjectId id;

	//get path
	const std::string & path = uri.getPath();

	//cases
	if (path == "auto") {
		const std::string & listing = uri.getParam("listing");
		const std::string & name = uri.getParam("name");
		id = this->objectIdListings.getObjectId(listing, name);
	} else if (sscanf(path.c_str(), "%ld:%ld", &id.high, &id.low) == 2) {
		//nothing more to do
	} else {
		UMMAP_FATAL_ARG("Invalid object ID in mero URI : %1")
			.arg(uri.getURI())
			.end();
	}

	//ret
	return id;
}

/*******************  FUNCTION  *********************/
//ioc://1234:1234
Driver * UriHandler::buildDriverIoc(const Uri & uri)
{
	//check
	assert(uri.getType() == "meroioc" || uri.getType() == "clovisioc" || uri.getType() == "ioc" || uri.getType() == "iocfile");

	//id
	ObjectId id = this->getIocObjectId(uri);

	//init mero
	this->ressourceHandler.checkRessource<IocRessource>("ioc");

	//build driver
	#ifdef HAVE_IOC_CLIENT
		return new IocDriver(IocRessource::getClient(), id.high, id.low);
	#else
		if (uri.getType() == "iocfile")
		{
			//warn
			UMMAP_WARNING("Mero is not available, using fake fopen mode for tests");

			//replacement
			char fname[1024];
			sprintf(fname, "%lx:%lx", id.high, id.low);
			return buildDriverFOpen(fname, "");
		} else {
			UMMAP_FATAL_ARG("Mero is not available, cannot use uri : %1").arg(uri.getURI()).end();
			return NULL;
		}
	#endif
}

/*******************  FUNCTION  *********************/
void UriHandler::initMero(const std::string & ressourceFile, int ressourceIndex)
{
	MeroRessource::setRessourceInfo(ressourceFile, ressourceIndex);
	this->ressourceHandler.checkRessource<MeroRessource>("mero");
}
