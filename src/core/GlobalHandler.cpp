/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//config
#include "config.h"
//unix
#include <cstdint>
#include <cassert>
#include <signal.h>
//local
#ifdef HAVE_HTOPML
#include "../htopml/HtopmlMappings.hpp"
#endif //HAVE_HTOPML
#include "../common/Debug.hpp"
#include "GlobalHandler.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/********************  MACROS  **********************/
#define GET_REG_ERR(context) ((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR]

/********************  GLOBAL  **********************/
static GlobalHandler * gblHandler = NULL;
static struct sigaction gblOldHandler;

/*******************  FUNCTION  *********************/
void ummapio::setupSegfaultHandler(void)
{
	//get old action
	sigaction(SIGSEGV, NULL, &gblOldHandler);

	//struct
	struct sigaction sa; 
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = segfaultHandler;

	//mask
	sigfillset(&sa.sa_mask);
	
	//reg
	sigaction(SIGSEGV, &sa, NULL);
}

/*******************  FUNCTION  *********************/
void ummapio::unsetSegfaultHandler(void)
{
	//mask
	sigfillset(&gblOldHandler.sa_mask);
	
	//reg
	sigaction(SIGSEGV, &gblOldHandler, NULL);
}

/*******************  FUNCTION  *********************/
void ummapio::segfaultHandler(int sig, siginfo_t *si, void *context)
{
	//extract
	void* addr         = (void*)si->si_addr;
	PageFaultType type = (PageFaultType)((GET_REG_ERR(context) & 2) >> 1);
	uint8_t   isWrite  = (type == PAGEFAULT_WRITE);

	//transmit
	if (gblHandler->onSegFault(addr, isWrite) == false)
		gblOldHandler.sa_sigaction(sig, si, context);
}

/*******************  FUNCTION  *********************/
GlobalHandler::GlobalHandler(void)
{
	#ifdef HAVE_HTOPML
	HtopmlMappingsHttpNode::registerMapping(&mappingRegistry);
	#endif //HAVE_HTOPML
}

/*******************  FUNCTION  *********************/
GlobalHandler::~GlobalHandler(void)
{
	if (this->mappingRegistry.isEmpty() == false) {
		UMMAP_WARNING("CAUTION: stopping ummap environnement while still having mapping, they will be destroyed.");
		this->mappingRegistry.deleteAllMappings();
	}
}

/*******************  FUNCTION  *********************/
void GlobalHandler::deleteAllMappings(void)
{
	this->mappingRegistry.deleteAllMappings();
}

/*******************  FUNCTION  *********************/
void GlobalHandler::registerMapping(Mapping * mapping)
{
	this->mappingRegistry.registerMapping(mapping);
}

/*******************  FUNCTION  *********************/
void GlobalHandler::unregisterMapping(Mapping * mapping)
{
	this->mappingRegistry.unregisterMapping(mapping);
}

/*******************  FUNCTION  *********************/
void GlobalHandler::registerPolicy(const std::string & name, Policy * policy)
{
	this->policyRegistry.registerPolicy(name, policy);
}

/*******************  FUNCTION  *********************/
void GlobalHandler::unregisterPolicy(const std::string & name)
{
	this->policyRegistry.unregisterPolicy(name);
}

/*******************  FUNCTION  *********************/
Policy * GlobalHandler::getPolicy(const std::string & name)
{
	return this->policyRegistry.get(name);
}

/*******************  FUNCTION  *********************/
bool GlobalHandler::onSegFault(void * addr, bool isWrite)
{
	//get mapping
	Mapping * mapping = this->mappingRegistry.getMapping(addr);

	//chec
	if (mapping == NULL) {
		UMMAP_WARNING_ARG("Unknown ummap mapping for the faulting address %1")
			.arg(addr)
			.end();
		return false;
	} else {
		//fault handling
		mapping->onSegmentationFault(addr, isWrite);
		return true;
	}
}

/*******************  FUNCTION  *********************/
void * GlobalHandler::ummap(size_t size, size_t segmentSize, size_t storageOffset, int protection, int flags, Driver * driver, Policy * localPolicy, const std::string & policyGroup)
{
	//get policy
	Policy * globalPolicy = NULL;
	if (policyGroup != "none")
	{
		globalPolicy = this->policyRegistry.get(policyGroup);
		assumeArg(globalPolicy != NULL, "Fail to find requested policy group : %1").arg(policyGroup).end();
	}

	//create mapping
	Mapping * mapping = new Mapping(size, segmentSize, storageOffset, protection, driver, localPolicy, globalPolicy);
	this->mappingRegistry.registerMapping(mapping);

	//no first read
	if (flags &= UMMAP_NO_FIRST_READ)
		mapping->skipFirstRead();
	
	//return
	return mapping->getAddress();
}

/*******************  FUNCTION  *********************/
int GlobalHandler::umunmap(void * ptr, bool sync)
{
	//get mapping
	Mapping * mapping = this->mappingRegistry.getMapping(ptr);

	//error
	assumeArg(mapping != NULL, "Fail to find ummap mapping to unmap : %1").arg(ptr).end();

	//unmap
	this->mappingRegistry.unregisterMapping(mapping);

	//sync
	if (sync)
		mapping->sync(0, mapping->getAlignedSize(), false);

	//delete
	delete mapping;

	//ok
	return 0;
}

/*******************  FUNCTION  *********************/
void GlobalHandler::skipFirstRead(void * ptr)
{
	//get mapping
	Mapping * mapping = this->mappingRegistry.getMapping(ptr);

	//error
	assumeArg(mapping != NULL, "Fail to find ummap mapping to unmap : %1").arg(ptr).end();

	//apply
	mapping->skipFirstRead();
}

/*******************  FUNCTION  *********************/
UriHandler & GlobalHandler::getUriHandler(void)
{
	return this->uriHandler;
}

/*******************  FUNCTION  *********************/
void GlobalHandler::flush(void * ptr, size_t size, bool evict)
{
	//get mapping
	Mapping * mapping = this->mappingRegistry.getMapping(ptr);

	//error
	assumeArg(mapping != NULL, "Fail to find ummap mapping to unmap : %1").arg(ptr).end();

	//compute
	size_t offset = (char*)ptr - (char*)mapping->getAddress();
	size_t segmentSize = mapping->getSegmentSize();

	//printf("flush internal %lu -> %lu -> %lu\n", offset, size, mapping->getAlignedSize());

	//size
	if (size == 0)
		size = mapping->getAlignedSize() - offset;
	
	//align
	if (offset % segmentSize != 0) {
		size_t delta = offset - offset % segmentSize;
		offset -= delta;
		size += delta;
	}
	if (size % segmentSize != 0) {
		size += segmentSize - offset % segmentSize;
	}

	//printf("flush internal %lu -> %lu -> %lu\n", offset, size, mapping->getAlignedSize());

	//check
	assume((char*)ptr + size <= (char*)mapping->getAddress() + mapping->getAlignedSize(),
		"Invalid flush size, not fit in ummap mapping !");

	//apply
	mapping->sync(offset, size, evict);
}

/*******************  FUNCTION  *********************/
void ummapio::setGlobalHandler(GlobalHandler * handler)
{
	assert(handler != NULL);
	assume(gblHandler == NULL, "Try to re-init ummap-io without previously destroy states !");
	gblHandler = handler;
}

/*******************  FUNCTION  *********************/
void ummapio::clearGlobalHandler(void)
{
	if (gblHandler != NULL) {
		delete gblHandler;
		gblHandler = NULL;
	}
}

/*******************  FUNCTION  *********************/
GlobalHandler * ummapio::getGlobalhandler(void)
{
	assume(gblHandler != NULL, "Missing initialization of ummap-io before use !");
	return gblHandler;
}
