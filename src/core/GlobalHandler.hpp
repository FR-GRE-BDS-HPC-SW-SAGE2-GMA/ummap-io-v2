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
class Handler
{
	public:
		Handler(Policy * globalPolicy);
		~Handler(void);
		void registerMapping(Mapping * mapping);
		void onSegFault(void * addr, bool isWrite);
		void deleteAllMappings(void);
	private:
		Registry mappingRegistry;
		Policy * globalPolicy;
};

/********************* GLOBAL  **********************/
extern Handler * gblHandler;

/*******************  FUNCTION  *********************/
void setupSegfaultHandler(void);
void segfaultHandler(int sig, siginfo_t *si, void *context);

}

#endif //UMMAP_GLOBAL_HANDLER_HPP
