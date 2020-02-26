/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_UNIX_OS_HPP
#define UMMAP_UNIX_OS_HPP

/********************  HEADERS  *********************/
#include <string>

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  STRUCT  *********************/
struct UnixOS
{
	static void mmap(void);
};

}

#endif //UMMAP_UNIX_OS_HPP
