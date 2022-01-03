/*****************************************************
			 PROJECT  : ummap-io-v2
			 VERSION  : 0.0.0-dev
			 DATE     : 04/2020
			 LICENSE  : ????????
*****************************************************/

/********************  HEADERS  *********************/
//#define _GNU_SOURCE
#include <dlfcn.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include "core/GlobalHandler.hpp"

/********************  FLAGS  **********************/
/**
 * Keep 1MB under our hands to force the compiler to read the data
 * and push it to this dummy segment.
**/
#define UMMAP_STORE_SIZE (1024*1024)

/*******************  GLOBALS  *********************/
/**
 * Keep 1MB under our hands to force the compiler to read the data
 * and push it to this dummy segment.
**/
char libWriteGblStore[UMMAP_STORE_SIZE];

/********************  TYPES  **********************/
/**
 * Signature of the system write operation to be able to use the
 * function extracted by dlsym().
**/
typedef ssize_t (*ummap_libc_write_t)(int fd, const void *buf, size_t count);

/*******************  FUNCTION  *********************/
//export as C interface
extern "C" {
	ssize_t write(int fd, const void *buf, size_t count);
}

/*******************  FUNCTION  *********************/
/**
 * Make a read first touch access on the segment to force ummap to map it.
**/
static void ummap_read_touch(const void * buf, size_t size)
{
	//special
	if (size == 0)
		return;

	//cast
	const char * cast_buffer = (const char *)buf;

	//touch one element per page
	size_t i;
	for (i = 0 ; i < size ; i += 4096)
		libWriteGblStore[i % UMMAP_STORE_SIZE] = cast_buffer[i];
	
	//touch last
	i = size - 1;
	libWriteGblStore[i % UMMAP_STORE_SIZE] = cast_buffer[i];
}

/*******************  FUNCTION  *********************/
ssize_t write(int fd, const void *buf, size_t count)
{
	//cast for easer user
	void * buffer = (void*)buf;

	//extract the original write symbol
	static ummap_libc_write_t libc_write = NULL;
	if (libc_write == NULL) {
		libc_write = (ummap_libc_write_t)dlsym(RTLD_NEXT, "write");
		assert(libc_write != NULL);
	}

	//make a first try
	ssize_t res = libc_write(fd, buffer, count);

	//not ok => need to prefetch
	if (res == -1) {
		//get mapping and check it is backed to ummap-io
		ummapio::Mapping * mapping = ummapio::getGlobalhandler()->getMapping(buffer);

		//if not, it is not our problem, forward the error to the caller
		if (mapping == NULL)
			return -1;

		//get the max memory from the mapping policies
		size_t policy_max = mapping->getPolicyMaxMemory();

		//calc what to prefetch
		size_t fetch = std::min(count, policy_max);
		
		//prefetch by reading one element per page
		ummap_read_touch(buffer, fetch);

		//call again write
		res = libc_write(fd, buffer, count);

		//debug
		//fprintf(stderr, "============> CAPTURED %p => %zu => %zd <============\n", buf, count, res);
	}
	return res;
}
