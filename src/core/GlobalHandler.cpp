/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//unix
#include <cstdint>
#include <cassert>
#include <signal.h>
//local
#include "../common/Debug.hpp"
#include "GlobalHandler.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap_io;

/********************  MACROS  **********************/
#define GET_REG_ERR(context) ((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR]

/********************  GLOBAL  **********************/
GlobalHandler * ummap_io::gblHandler = NULL;
static void (*gblOldHandler) (int, siginfo_t *, void *) = NULL;

/*******************  FUNCTION  *********************/
void ummap_io::setupSegfaultHandler(void)
{
	//get old action
	struct sigaction oldAction;
	sigaction(SIGSEGV, NULL, &oldAction);
	gblOldHandler = oldAction.sa_sigaction;

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
void ummap_io::unsetSegfaultHandler(void)
{
	signal(SIGSEGV, SIG_DFL);
}

/*******************  FUNCTION  *********************/
void ummap_io::segfaultHandler(int sig, siginfo_t *si, void *context)
{
	//extract
	void* addr         = (void*)si->si_addr;
    PageFaultType type = (PageFaultType)((GET_REG_ERR(context) & 2) >> 1);
    uint8_t   isWrite  = (type == PAGEFAULT_WRITE);

	//transmit
	if (gblHandler->onSegFault(addr, isWrite) == false)
		if (gblOldHandler != NULL)
			gblOldHandler(sig, si, context);
}

/*******************  FUNCTION  *********************/
GlobalHandler::GlobalHandler(void)
{
}

/*******************  FUNCTION  *********************/
GlobalHandler::~GlobalHandler(void)
{
	if (this->mappingRegistry.isEmpty() == false) {
		UMMAP_WARNING("CAUTION: stopping ummap environnement while still having mapping, they will be destroyed.");
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

void GlobalHandler::unregisterMapping(Mapping * mapping)
{
	this->mappingRegistry.unregisterMapping(mapping);
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
void ummap_io::setGlobalHandler(GlobalHandler * handler)
{
	assert(handler != NULL);
	gblHandler = handler;
}

/*******************  FUNCTION  *********************/
void ummap_io::clearGlobalHandler(void)
{
	if (gblHandler != NULL) {
		delete gblHandler;
		gblHandler = NULL;
	}
}
