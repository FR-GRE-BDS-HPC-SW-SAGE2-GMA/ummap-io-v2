/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_GLOBAL_HANDLER_HPP
#define UMMAP_GLOBAL_HANDLER_HPP

/********************  HEADERS  *********************/
#include "Registry.hpp"
#include "Policy.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  TYPES  **********************/
enum PageFaultType
{
    PAGEFAULT_READ = 0,
    PAGEFAULT_WRITE
};

/*********************  CLASS  **********************/
class GlobalHandler
{
	public:
		GlobalHandler(Policy * globalPolicy);
		~GlobalHandler(void);
		void registerMapping(Mapping * mapping);
		bool onSegFault(void * addr, bool isWrite);
		void deleteAllMappings(void);
		void setGlobalPolicy(Policy * policy);
	private:
		Registry mappingRegistry;
		Policy * globalPolicy;
};

/********************* GLOBAL  **********************/
extern GlobalHandler * gblHandler;

/*******************  FUNCTION  *********************/
void setupSegfaultHandler(void);
void unsetSegfaultHandler(void);
void segfaultHandler(int sig, siginfo_t *si, void *context);
void setGlobalHandler(GlobalHandler * handler);
void clearGlobalHandler(void);

}

#endif //UMMAP_GLOBAL_HANDLER_HPP
