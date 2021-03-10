/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_GLOBAL_HANDLER_HPP
#define UMMAP_GLOBAL_HANDLER_HPP

/********************  HEADERS  *********************/
#include <map>
#include <signal.h>
#include "MappingRegistry.hpp"
#include "PolicyRegistry.hpp"
#include "../uri/UriHandler.hpp"
#include "../public-api/ummap.h"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  TYPES  **********************/
/**
 * Used to track the page fault type in a readable way.
**/
enum PageFaultType
{
	/** The page fault is a read fault. **/
	PAGEFAULT_READ = 0,
	/** The page fault is a write fault. **/
	PAGEFAULT_WRITE
};

/*********************  CLASS  **********************/
/**
 * The global handler is the main struct handling all the objects to make
 * ummap working. It is called from the C public API.
**/
class GlobalHandler
{
	public:
		GlobalHandler(void);
		~GlobalHandler(void);
		void * ummap(void * addr, size_t size, size_t segmentSize, size_t storageOffset, int protection, int flags, Driver * driver, Policy * localPolicy, const std::string & policyGroup);
		int uunmap(void * ptr, bool sync);
		void flush(void * ptr, size_t size, bool evict);
		void skipFirstRead(void * ptr);
		void registerPolicy(const std::string & name, Policy * policy);
		void unregisterPolicy(const std::string & name);
		Policy * getPolicy(const std::string & name);
		void registerMapping(Mapping * mapping);
		void unregisterMapping(Mapping * mapping);
		bool onSegFault(void * addr, bool isWrite);
		void deleteAllMappings(void);
		UriHandler & getUriHandler(void);
		Mapping * getMapping(void * addr);
	private:
		/** Registry of all active mappings in use. **/
		MappingRegistry mappingRegistry;
		/** Registry of global policies in use. **/
		PolicyRegistry policyRegistry;
		/** URI handler to be used to build drivers and policies from strings. **/
		UriHandler uriHandler;
};

/*******************  FUNCTION  *********************/
//fault handler
void setupSegfaultHandler(void);
void unsetSegfaultHandler(void);
void segfaultHandler(int sig, siginfo_t *si, void *context);

/*******************  FUNCTION  *********************/
//global handler
void setGlobalHandler(GlobalHandler * handler);
void clearGlobalHandler(void);
GlobalHandler * getGlobalhandler(void);

}

#endif //UMMAP_GLOBAL_HANDLER_HPP
