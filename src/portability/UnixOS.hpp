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

#define UMMAP_PAGE_SIZE 4096

/*********************  STRUCT  *********************/
struct UnixOS
{
	static void * mmapProtNone(size_t size);
	static void munmap(void * ptr, size_t size);
	static int cpuNumber(void);
};

}

#endif //UMMAP_UNIX_OS_HPP
