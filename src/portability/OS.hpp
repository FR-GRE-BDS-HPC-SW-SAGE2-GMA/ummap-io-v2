/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_OS_HPP
#define UMMAP_OS_HPP

/********************  HEADERS  *********************/
#include "config.h"

/********************  HEADERS  *********************/
#ifdef UMMAP_PORTABILITY_OS_UNIX
	#include "UnixOS.hpp"
	namespace ummapio
	{
		typedef UnixOS OS;
	}
#else
	#error Unsupported portability mode for OS, should be UNIX
#endif

#endif //UMMAP_OS_HPP
