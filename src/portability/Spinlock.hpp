/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_SPINLOCK_HPP
#define UMMAP_SPINLOCK_HPP

/********************  INFO   ***********************/
/**
 * This file is imported from project DAQ developped by Sebastien Valat.
 * at exascale lab / university of versailles
**/

/********************  HEADERS  *********************/
//must be on top
#include "config.h"

/*********************  TYPES  **********************/
#if defined(UMMAP_PORTABILITY_SPINLOCK_PTHREAD)
	//pthread mode
	#include "SpinlockPthread.hpp"

	//map types to generic names
	namespace ummap_io
	{
		typedef SpinlockPthread Spinlock;
	}
#elif defined(UMMAP_PORTABILITY_SPINLOCK_APPLE)
	//pthread mode
	#include "SpinlockApple.hpp"

	//map types to generic names
	namespace ummap
	{
		typedef SpinlockApple Spinlock;
	}
#elif defined(UMMAP_PORTABILITY_SPINLOCK_DUMMY)
	//dummy mode (not thread safe, only for quik portability)
	#include "LockDummy.hpp"
	
	//show some warning
	#warning Caution, you are using the DUMMY mutex implementation, UMMAP-IO will not be thread-safe !

	//map types to generic names
	namespace ummap
	{
		typedef SpinlockDummy Spinlock;
	};
#else
	//not found, fail to compile
	#error "No available implementation for mutex, please check definition of one of UMMAP_PORTABILITY_SPINLOCK_* macro in config.h or PORTABILITY_SPINLOCK given to cmake."
#endif

#endif //UMMAP_SPINLOCK_HPP
