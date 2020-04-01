/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include "../core/GlobalHandler.hpp"
#include "ummap.h"

/***************** USING NAMESPACE ******************/
using namespace ummap_io;

/*******************  FUNCTION  *********************/
void ummap_init(void)
{
	ummap_io::GlobalHandler * handler = new ummap_io::GlobalHandler();
	ummap_io::setGlobalHandler(handler);
	ummap_io::setupSegfaultHandler();
}

/*******************  FUNCTION  *********************/
void ummap_finalize(void)
{
	ummap_io::unsetSegfaultHandler();
	ummap_io::clearGlobalHandler();
}
