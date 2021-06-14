/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_GLOBAL_HANDLER_HPP
#define UMMAP_GLOBAL_HANDLER_HPP

/********************  HEADERS  *********************/
//std
#include <cassert>
#include <map>
#include <functional>
//unix
#include <signal.h>
//internal
#include "common/Debug.hpp"
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
		void flush(void * ptr, size_t size, bool evict, bool sync);
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
		template <class T> int applyCow(const char * driverName, void * addr, std::function<int(Mapping * mapping, T * driver)> action);
		template <class T> int applySwitch(const char * driverName, void * addr, ummap_switch_clean_t cleanAction, std::function<void(T * driver)> action);
		void initMero(const std::string & ressourceFile, int ressourceIndex);
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

/*******************  FUNCTION  *********************/
template <class T> 
int GlobalHandler::applyCow(const char * driverName, void * addr, std::function<int(Mapping * mapping, T * driver)> action)
{
	//check
	assert(addr != NULL);

	//get mapping
	Mapping * mapping = getGlobalhandler()->getMapping(addr);
	assumeArg(mapping != NULL, "Fail to find the requested mapping for address %p !").arg(addr).end();

	//get the driver
	Driver * driver = mapping->getDriver();
	assume(driver != NULL, "Get an unknown NULL driver !");

	//try to cast to IOC driver
	T * castDriver = dynamic_cast<T*>(driver);
	assumeArg(castDriver != NULL, "Get an invalid unknown driver type, not %1, cannot COW !").arg(driverName).end();

	//call copy on write on the driver.
	mapping->unregisterRange();
	int status = action(mapping, castDriver);
	mapping->registerRange();

	//return
	return status;
}

template <class T>
int GlobalHandler::applySwitch(const char * driverName, void * addr, ummap_switch_clean_t cleanAction, std::function<void(T * driver)> action)
{
	//check
	assert(addr != NULL);

	//get mapping
	Mapping * mapping = getGlobalhandler()->getMapping(addr);
	assumeArg(mapping != NULL, "Fail to find the requested mapping for address %p !").arg(addr).end();

	//get the driver
	Driver * driver = mapping->getDriver();
	assume(driver != NULL, "Get an unknown NULL driver !");

	//try to cast to IOC driver
	T * castDriver = dynamic_cast<T*>(driver);
	assumeArg(castDriver != NULL, "Get an invalid unknown driver type, not %1, cannot COW !").arg(driverName).end();

	//call copy on write on the driver.
	mapping->unregisterRange();
	action(castDriver);
	mapping->registerRange();

	//if drop
	switch (cleanAction) {
		case UMMAP_NO_ACTION:
			break;
		case UMMAP_DROP_CLEAN:
			mapping->dropClean();
			break;
		case UMMAP_MARK_CLEAN_DIRTY:
			mapping->markCleanAsDirty();
			break;
		default:
			UMMAP_FATAL_ARG("Invalid clean action for switch operation, got %1").arg(cleanAction).end();
			break;
	}
		

	//return
	return 0;
}

}

#endif //UMMAP_GLOBAL_HANDLER_HPP
