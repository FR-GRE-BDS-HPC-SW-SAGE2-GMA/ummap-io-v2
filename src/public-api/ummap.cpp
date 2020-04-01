/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include "../core/GlobalHandler.hpp"
#include "ummap.h"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
void ummap_init(void)
{
	ummapio::GlobalHandler * handler = new ummapio::GlobalHandler();
	ummapio::setGlobalHandler(handler);
	ummapio::setupSegfaultHandler();
}

/*******************  FUNCTION  *********************/
void ummap_finalize(void)
{
	ummapio::unsetSegfaultHandler();
	ummapio::clearGlobalHandler();
}
