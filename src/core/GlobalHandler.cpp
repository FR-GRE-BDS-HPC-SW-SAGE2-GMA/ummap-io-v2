/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//unix
#include <cstdint>
#include <signal.h>
//local
#include "../common/Debug.hpp"
#include "GlobalHandler.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/********************  MACROS  **********************/
#define GET_REG_ERR(context) ((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR]

/********************  GLOBAL  **********************/
Handler * ummap::gblHandler = NULL;

/*******************  FUNCTION  *********************/
void ummap::setupSegfaultHandler(void)
{
	//struct
	struct sigaction sa = { 
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = segfaultHandler,
	};

	//mask
	sigfillset(&sa.sa_mask);
	
	//reg
	sigaction(SIGSEGV, &sa, NULL);
}

/*******************  FUNCTION  *********************/
void ummap::segfaultHandler(int sig, siginfo_t *si, void *context)
{
	//extract
	void* addr         = (void*)si->si_addr;
    PageFaultType type = (PageFaultType)((GET_REG_ERR(context) & 2) >> 1);
    uint8_t   isWrite  = (type == PAGEFAULT_WRITE);

	//transmit
	gblHandler->onSegFault(addr, isWrite);
}

/*******************  FUNCTION  *********************/
Handler::Handler(Policy * globalPolicy)
{
	this->globalPolicy = globalPolicy;
}

/*******************  FUNCTION  *********************/
Handler::~Handler(void)
{
}

/*******************  FUNCTION  *********************/
void Handler::deleteAllMappings(void)
{
	this->mappingRegistry.deleteAllMappings();
}

/*******************  FUNCTION  *********************/
void Handler::registerMapping(Mapping * mapping)
{
	this->mappingRegistry.registerMapping(mapping);
}

/*******************  FUNCTION  *********************/
void Handler::onSegFault(void * addr, bool isWrite)
{
	//get mapping
	Mapping * mapping = this->mappingRegistry.getMapping(addr);

	//chec
	assumeArg(mapping != NULL, "Unknown ummap mapping for the faulting address %1")
		.arg(addr)
		.end();

	//fault handling
	mapping->onSegmentationFault(addr, isWrite);
}
