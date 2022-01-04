/*****************************************************
			 PROJECT  : ummap-io-v2
			 VERSION  : 0.0.0-dev
			 DATE     : 04/2020
			 LICENSE  : ????????
*****************************************************/

/**
 * @todo Make a better implementation by taking lock, mapping the required memory, makeing 
 * rw then unlocking. This might be safer than the current implementation.
**/

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
typedef ssize_t (*ummap_libc_rw_t)(int fd, const void *buf, size_t count);

/*******************  FUNCTION  *********************/
//export as C interface
extern "C" {
	ssize_t write(int fd, const void *buf, size_t count);
}

/*******************  FUNCTION  *********************/
/**
 * Make a read first touch access on the segment to force ummap to map it.
**/
static void ummap_touch(ummapio::Mapping & mapping, const void * buf, size_t size, bool isWrite)
{
	//special
	if (size == 0)
		return;

	//cast
	char * cast_buffer = (char *)buf;

	//touch one element per page
	size_t i;
	for (i = 0 ; i < size ; i += 4096)
		mapping.onSegmentationFault(cast_buffer+i, isWrite);
	
	//touch last
	mapping.onSegmentationFault(cast_buffer + size - 1, isWrite);
}

/*******************  FUNCTION  *********************/
ssize_t write(int fd, const void *buf, size_t count)
{
	//cast for easer user
	void * buffer = (void*)buf;

	//extract the original write symbol
	static ummap_libc_rw_t libc_write = NULL;
	if (libc_write == NULL) {
		libc_write = (ummap_libc_rw_t)dlsym(RTLD_NEXT, "write");
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
		ummap_touch(*mapping, buffer, fetch, false);

		//call again write
		res = libc_write(fd, buffer, count);

		//debug
		//fprintf(stderr, "============> CAPTURED %p => %zu => %zd <============\n", buf, count, res);
	}
	return res;
}

/*******************  FUNCTION  *********************/
ssize_t read(int fd, const void *buf, size_t count)
{
	//cast for easer user
	void * buffer = (void*)buf;

	//extract the original read symbol
	static ummap_libc_rw_t libc_read = NULL;
	if (libc_read == NULL) {
		libc_read = (ummap_libc_rw_t)dlsym(RTLD_NEXT, "read");
		assert(libc_read != NULL);
	}

	//make a first try
	ssize_t res = libc_read(fd, buffer, count);

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
		ummap_touch(*mapping, buffer, fetch, false);

		//call again read
		res = libc_read(fd, buffer, count);

		//debug
		//fprintf(stderr, "============> CAPTURED %p => %zu => %zd <============\n", buf, count, res);
	}
	return res;
}
