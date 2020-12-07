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
/**
 * Extract faulting error informations. It is used in the page fault handler
 * to extract the type of fault.
**/
#define GET_REG_ERR(context) ((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR]

/********************  GLOBAL  **********************/
/**
 * The global handler is spawned once and tracked by this global pointer.
**/
static GlobalHandler * gblHandler = NULL;
/**
 * When setting the segmentation fault handler we want to keep track
 * of a possibly pre-existing one so we can fallback on it if the fault
 * is not for us.
**/
static struct sigaction gblOldHandler;

/*******************  FUNCTION  *********************/
/**
 * Function used to setup the page fault handler and keep track
 * of the old one.
**/
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
/**
 * Unset the page fault handler and restore the old one.
**/
void ummapio::unsetSegfaultHandler(void)
{
	//mask
	sigfillset(&gblOldHandler.sa_mask);
	
	//reg
	sigaction(SIGSEGV, &gblOldHandler, NULL);
}

/*******************  FUNCTION  *********************/
/**
 * C function used to capture segmentation faults.
 * @param sig Signal recived.
 * @param si Signal informations.
 * @param context Context of the fault.
**/
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
/**
 * Constructor of the global handler.
 * If we use htopml this functions is responsible of registering the
 * mapping registry to htopml in order to get access to all pending
 * mappings.
**/
GlobalHandler::GlobalHandler(void)
{
	#ifdef HAVE_HTOPML
	HtopmlMappingsHttpNode::registerMapping(&mappingRegistry);
	#endif //HAVE_HTOPML
}

/*******************  FUNCTION  *********************/
/**
 * Global destructor responsible of destroying all active mappings
 * before existing.
**/
GlobalHandler::~GlobalHandler(void)
{
	if (this->mappingRegistry.isEmpty() == false) {
		UMMAP_WARNING("CAUTION: stopping ummap environnement while still having mapping, they will be destroyed.");
		this->mappingRegistry.deleteAllMappings();
	}
}

/*******************  FUNCTION  *********************/
/**
 * Delete all active mappings. This is used buy the destructor.
**/
void GlobalHandler::deleteAllMappings(void)
{
	this->mappingRegistry.deleteAllMappings();
}

/*******************  FUNCTION  *********************/
/**
 * Register a new mapping to the global handler to be notified
 * on segmentation fault.
 * @param mapping Pointer to the mapping to register.
**/
void GlobalHandler::registerMapping(Mapping * mapping)
{
	this->mappingRegistry.registerMapping(mapping);
}

/*******************  FUNCTION  *********************/
/**
 * Unregister the given mapping before destroying it so it does
 * not recive anymore segmentation fault notifications.
 * @param mapping Pointer to the mapping to unregister.
**/
void GlobalHandler::unregisterMapping(Mapping * mapping)
{
	this->mappingRegistry.unregisterMapping(mapping);
}

/*******************  FUNCTION  *********************/
/**
 * Register a global policy to the policy registry.
 * @param name Define the name of the policy.
 * @param policy Pointer to the policty to register.
**/
void GlobalHandler::registerPolicy(const std::string & name, Policy * policy)
{
	this->policyRegistry.registerPolicy(name, policy);
}

/*******************  FUNCTION  *********************/
/**
 * Unregister the policy identifed by its name. It will also destroy it/
 * @param name Name of the policy to unregister.
**/
void GlobalHandler::unregisterPolicy(const std::string & name)
{
	this->policyRegistry.unregisterPolicy(name);
}

/*******************  FUNCTION  *********************/
/**
 * Ask the policy registry to get the given policy identifed by its name.
 * @param name Name of the policy to extract.
 * @return Return a pointer to the given policy of NULL if not found.
**/
Policy * GlobalHandler::getPolicy(const std::string & name)
{
	return this->policyRegistry.get(name);
}

/*******************  FUNCTION  *********************/
/**
 * Handle segmentation fault signal to redirect the notification to the
 * related mapping.
 * @param addr Define the address of the fault.
 * @param isWrite True if the fault is a write access of false for a read access.
 * @return Return true if the notification has been delivered or false if not.
 * For non delivered message the caller must send the fault signal to the default
 * handler.
**/
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
/**
 * Establish a new memory mapping.
 * @param size Define the size of the mapping. If can be a non multiple of segment size. In this case, the mapping itself
 * will be sized to the next multiple. For read/write operations it will ignore this extra sub-segment.
 * @param segmentSize Equivalent of the page size for a standard mmap, it define the granularity of the IO operations.
 * This size must be a multiple of the OS page size (4K).
 * @param storageOffset Offset to apply on the storage of reach the data to be mapped. It does not have to be aligned on
 * page size.
 * @param protection Define the access protection to assign to this mapping. It uses the flags from mmap so you can
 * use the given flags and 'or' them: PROT_READ, PROT_WRIT, PROT_EXEC.
 * @param flags Flags to enable of disable some behaviors of ummap-io. Currently valid flags are : UMMAP_NO_FIRST_READ, 
 * UMMAP_THREAD_UNSAFE. Go in their respective documentation to get more information on them.
 * @param driver Pointer to the given driver. If UMMAP_DRVIER_NO_AUTO_DELETE if enabled the destruction of the driver is you 
 * own responsability, otherwise it will be destroyed automatically.
 * @param localPolicy Define the local policy to be used.
 * @param globalPolicy Define the global policy to be used and shared between multiple mappings.
**/
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
	if (flags & UMMAP_NO_FIRST_READ)
		mapping->skipFirstRead();

	//no thread sage
	if (flags & UMMAP_THREAD_UNSAFE)
		mapping->disableThreadSafety();
	
	//return
	return mapping->getAddress();
}

/*******************  FUNCTION  *********************/
/**
 * Unmap the mapping identifed by the given address.
 * @param ptr Address of the segment to unmap. It can be any bytes 
 * inside the targeted segments.
 * @param sync Apply a sync before unmapping the segment.
**/
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
/**
 * Notify the given segment to not read the storage data on the first memory access but init
 * the memory with zeroes.
 * @param ptr Pointer to any bytes inside the targeted segment.
**/
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
/**
 * Return a reference to the URI handler.
**/
UriHandler & GlobalHandler::getUriHandler(void)
{
	return this->uriHandler;
}

/*******************  FUNCTION  *********************/
/**
 * Apply a flush operation of the given segment. This will send
 * all the data to the storage.
 * @param ptr Define the base address from where to start the flush operation.
 * @param size Define the size of the region to flush.
 * @param evict Enable of disable the automatic eviction of the flushed pages.
**/
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
/**
 * Setup the global handler pointer.
 * @param handler Handler to register.
**/
void ummapio::setGlobalHandler(GlobalHandler * handler)
{
	assert(handler != NULL);
	assume(gblHandler == NULL, "Try to re-init ummap-io without previously destroy states !");
	gblHandler = handler;
}

/*******************  FUNCTION  *********************/
/**
 * For global fini operation, destroy the global handler so
 * all ressources will be cleaned.
**/
void ummapio::clearGlobalHandler(void)
{
	if (gblHandler != NULL) {
		delete gblHandler;
		gblHandler = NULL;
	}
}

/*******************  FUNCTION  *********************/
/**
 * Return the global handler.
**/
GlobalHandler * ummapio::getGlobalhandler(void)
{
	assume(gblHandler != NULL, "Missing initialization of ummap-io before use !");
	return gblHandler;
}
