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
namespace ummapio
{

#define UMMAP_PAGE_SIZE 4096

/*********************  STRUCT  *********************/
struct UnixOS
{
	static void * mmapProtFull(size_t size, bool exec);
	static void * mmapProtNone(size_t size);
	static void munmap(void * ptr, size_t size);
	static void mremapForced(void * oldPtr, size_t size, void * newPtr);
	static void mprotect(void * ptr, size_t size, bool read, bool write, bool exec);
	static void madviseDontNeed(void * ptr, size_t size);
	static int cpuNumber(void);
	static void removeFile(const std::string & path);
};

}

#endif //UMMAP_UNIX_OS_HPP
