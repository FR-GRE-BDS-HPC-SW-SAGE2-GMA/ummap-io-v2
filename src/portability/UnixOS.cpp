/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdio>
//unix
#include <sys/mman.h>
//internal
#include "../common/Debug.hpp"
//local
#include "UnixOS.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
void * UnixOS::mmapProtNone(size_t size)
{
	//check
	assert(size % UMMAP_PAGE_SIZE == 0);
	assert(size > 0);

	//call
	void * ptr = ::mmap(NULL, size, PROT_NONE, MAP_ANON | MAP_PRIVATE, 0, 0);

	//post check
	assumeArg(ptr != NULL, "Fail to call mmap with size=%1 : %2").arg(size).arg(strerror(errno)).end();

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
		.arg(strerror(errno))
		.end();
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
	assumeArg(fp != NULL, "Fail to open /proc/cpuinfo : %1").arg(strerror(errno)).end();

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
