/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdio>
//unix
#include <sys/mman.h>
#include <unistd.h>
//internal
#include "../common/Debug.hpp"
//local
#include "UnixOS.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
void * UnixOS::mmapProtNone(void * addr, size_t size, bool mapFixed)
{
	//check
	assert(size % UMMAP_PAGE_SIZE == 0);
	assert(size > 0);

	//flags
	int flags = MAP_ANON | MAP_PRIVATE;
	if (mapFixed)
		flags |= MAP_FIXED;

	//call
	void * ptr = ::mmap(addr, size, PROT_NONE, flags, 0, 0);

	//post check
	assumeArg(ptr != MAP_FAILED, "Fail to call mmap with size=%1 : %2").arg(size).argStrErrno().end();

	//ok
	return ptr;
}

/*******************  FUNCTION  *********************/
void * UnixOS::mmapProtFull(size_t size, bool exec)
{
	//check
	assert(size % UMMAP_PAGE_SIZE == 0);
	assert(size > 0);

	//prot
	int prot = PROT_READ | PROT_WRITE;
	if (exec)
		prot |= PROT_EXEC;

	//call
	void * ptr = ::mmap(NULL, size, prot, MAP_ANON | MAP_PRIVATE, 0, 0);

	//post check
	assumeArg(ptr != MAP_FAILED, "Fail to call mmap with size=%1 : %2").arg(size).argStrErrno().end();

	//ok
	return ptr;
}

/*******************  FUNCTION  *********************/
void UnixOS::munmap(void * ptr, size_t size)
{
	//check
	assert(ptr != NULL);
	assert(size % UMMAP_PAGE_SIZE == 0);
	assert(size > 0);

	//call
	int status = ::munmap(ptr, size);

	//post check
	assumeArg(status == 0, "Fail to call munmap with ptr=%1, size=%2 : %3")
		.arg(ptr)
		.arg(size)
		.argStrErrno()
		.end();
}

/*******************  FUNCTION  *********************/
void UnixOS::mremapForced(void * oldPtr, size_t size, void * newPtr)
{
	//check
	assert(newPtr != NULL);
	assert(oldPtr != NULL);
	assert((size_t)newPtr % UMMAP_PAGE_SIZE == 0);
	assert((size_t)oldPtr % UMMAP_PAGE_SIZE == 0);
	assert(size > 0);
	assert(size % UMMAP_PAGE_SIZE == 0);

	//trivial
	if (newPtr == oldPtr)
		return;

	//move
	void * res = mremap(oldPtr, size, size, MREMAP_FIXED | MREMAP_MAYMOVE, newPtr);
	assumeArg(res != MAP_FAILED, "Fail to call mremap : %1").argStrErrno().end();
	assume(res == newPtr, "Fail to mremap the segment to a given address, got a different one !");
}

/*******************  FUNCTION  *********************/
void UnixOS::mprotect(void * ptr, size_t size, bool read, bool write, bool exec)
{
	//checks
	assert(ptr != NULL);
	assert((size_t)ptr % UMMAP_PAGE_SIZE == 0);
	assert(size > 0);
	assert(size % UMMAP_PAGE_SIZE == 0);

	//prot
	int prot = 0;
	if (read)
		prot |= PROT_READ;
	if (write)
		prot |= PROT_WRITE;
	if (exec)
		prot |= PROT_EXEC;

	//apply
	::mprotect(ptr, size, prot);
}

/*******************  FUNCTION  *********************/
int UnixOS::cpuNumber(void)
{
	//static to go faster
	static int gblCpu = 0;

	//alreay scanned
	if (gblCpu > 0)
		return gblCpu;

	//open /proc/cpuinfo
	FILE * fp = fopen("/proc/cpuinfo", "r");
	assumeArg(fp != NULL, "Fail to open /proc/cpuinfo : %1").argStrErrno().end();

	//read until end
	char buffer[4096];
	int cnt = 0;
	while (!feof(fp)) {
		char * tmp = fgets(buffer, sizeof(buffer), fp);
		if (tmp != NULL) {
			int id = 0;
			if (sscanf(tmp, "processor       : %d",&id) == 1) {
				cnt = id;
			}
		}
	}

	//final
	gblCpu = cnt + 1;

	//ok
	return gblCpu;
}

/*******************  FUNCTION  *********************/
void UnixOS::madviseDontNeed(void * ptr, size_t size)
{
	madvise(ptr, size, MADV_DONTNEED);
}

/*******************  FUNCTION  *********************/
void UnixOS::removeFile(const std::string & path)
{
	int res = unlink(path.c_str());
	assumeArg(res == 0, "Fail to remove file %s : %s").arg(path).argStrErrno().end();
}
