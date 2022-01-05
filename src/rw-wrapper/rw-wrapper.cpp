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

/********************  TYPES  **********************/
/**
 * Signature of the system write operation to be able to use the
 * function extracted by dlsym().
**/
typedef ssize_t (*ummap_libc_write_t)(int fd, const void *buf, size_t count);
typedef ssize_t (*ummap_libc_read_t)(int fd, void *buf, size_t count);
typedef size_t (*ummap_libc_fread_t)(void *ptr, size_t size, size_t nmemb, FILE *stream);
typedef size_t (*ummap_libc_fwrite_t)(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/*******************  FUNCTION  *********************/
//export as C interface
extern "C" {
	ssize_t write(int fd, const void *buf, size_t count);
	ssize_t read(int fd, void *buf, size_t count);
	size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
	size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
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
	ssize_t i;
	for (i = size - 1 ; i >= 0 ; i -= 4096)
		mapping.onSegmentationFault(cast_buffer+i, isWrite);

	//touch first to be sure to succes
	mapping.onSegmentationFault(cast_buffer, isWrite);
}

/*******************  FUNCTION  *********************/
ssize_t write(int fd, const void *buf, size_t count)
{
	//cast for easer user
	char * buffer = (char*)buf;

	//extract the original write symbol
	static ummap_libc_write_t libc_write = NULL;
	if (libc_write == NULL) {
		libc_write = (ummap_libc_write_t)dlsym(RTLD_NEXT, "write");
		assumeArg(libc_write != NULL, "Fail to find the write symbol via dlsym() : %1")
			.argStrErrno().end();
	}

	//make a first try
	ssize_t res = libc_write(fd, buffer, count);

	//not ok => need to prefetch
	if (res == -1 && errno == EFAULT) {
		//get mapping and check it is backed to ummap-io
		ummapio::Mapping * mapping = ummapio::getGlobalhandler()->getMapping(buffer, false);

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
		do {
			res = libc_write(fd, buffer, count);
		} while (res == -1 && errno == EAGAIN);

		//recall
		if (res > 0 && res < count) {
			ssize_t res2 = write(fd, buffer + res, count - res);
			assumeArg(res2 >= 0, "Invalid status %1").arg(res2).end();
			res += res2;
		}

		//debug
		UMMAP_DEBUG_ARG("rw-wrapper", "Capture write %1 => %2 < %3 => %4")
			.arg(buf).arg(count).arg(fetch).arg(res).end();
	}
	return res;
}

/*******************  FUNCTION  *********************/
ssize_t read(int fd, void *buf, size_t count)
{
	//cast for easer user
	void * buffer = (void*)buf;

	//extract the original read symbol
	static ummap_libc_read_t libc_read = NULL;
	if (libc_read == NULL) {
		libc_read = (ummap_libc_read_t)dlsym(RTLD_NEXT, "read");
		assumeArg(libc_read != NULL, "Fail to find the read symbol via dlsym() : %1")
			.argStrErrno().end();
	}

	//make a first try
	ssize_t res = libc_read(fd, buffer, count);

	//not ok => need to prefetch
	if (res == -1 && errno == EFAULT) {
		//get mapping and check it is backed to ummap-io
		ummapio::Mapping * mapping = ummapio::getGlobalhandler()->getMapping(buffer, false);

		//if not, it is not our problem, forward the error to the caller
		if (mapping == NULL)
			return -1;

		//get the max memory from the mapping policies
		size_t policy_max = mapping->getPolicyMaxMemory();

		//calc what to prefetch
		size_t fetch = std::min(count, policy_max);

		//prefetch by reading one element per page
		ummap_touch(*mapping, buffer, fetch, true);

		//call again read
		res = libc_read(fd, buffer, count);

		//debug
		UMMAP_DEBUG_ARG("rw-wrapper", "Capture read %1 => %2 < %3 => %4")
			.arg(buf).arg(count).arg(fetch).arg(res).end();
	}
	return res;
}

/*******************  FUNCTION  *********************/
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	//cast for easer user
	char * buffer = (char*)ptr;

	//extract the original fread symbol
	static ummap_libc_fread_t libc_fread = NULL;
	if (libc_fread == NULL) {
		libc_fread = (ummap_libc_fread_t)dlsym(RTLD_NEXT, "fread");
		assumeArg(libc_fread != NULL, "Fail to find the fread symbol via dlsym() : %1")
			.argStrErrno().end();
	}

	//make a first try
	ssize_t res = libc_fread(ptr, size, nmemb, stream);

	//not ok => need to prefetch
	if (res == -1 && errno == EFAULT) {
		//get mapping and check it is backed to ummap-io
		ummapio::Mapping * mapping = ummapio::getGlobalhandler()->getMapping(buffer, false);

		//if not, it is not our problem, forward the error to the caller
		if (mapping == NULL)
			return -1;

		//get the max memory from the mapping policies
		size_t policy_max = mapping->getPolicyMaxMemory();

		//calc what to prefetch
		size_t fetch = std::min(size*nmemb, policy_max);
		fetch -= fetch % size;
		assumeArg(fetch > size, "Invalid fetch (%1) smaller than size (%2)")
			.arg(fetch)
			.arg(size)
			.end();

		//prefetch by reading one element per page
		ummap_touch(*mapping, buffer, fetch, true);

		//call again read
		size_t fetch_nmemb = fetch / size;
		res = libc_fread(ptr, size, fetch_nmemb, stream);

		//debug
		UMMAP_DEBUG_ARG("rw-wrapper", "Capture fread %1 => %2 => %3")
			.arg(buffer).arg(nmemb).arg(res).end();

		//recall
		if (res > 0 && res < nmemb) {
			//call again
			ssize_t res2 = fread(buffer + res*size, size, nmemb - res, stream);
			assumeArg(res2 >= 0, "Invalid status %1").arg(res2).end();
			res += res2;

			//debug
			UMMAP_DEBUG_ARG("rw-wrapper", "Capture fread recall %1 => %2 => %3")
				.arg(buffer).arg(nmemb).arg(res).end();
		}
	}
	return res;
}

/*******************  FUNCTION  *********************/
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	//cast for easer user
	char * buffer = (char*)ptr;

	//extract the original fwrite symbol
	static ummap_libc_fwrite_t libc_fwrite = NULL;
	if (libc_fwrite == NULL) {
		libc_fwrite = (ummap_libc_fwrite_t)dlsym(RTLD_NEXT, "fwrite");
		assumeArg(libc_fwrite != NULL, "Fail to find the fwrite symbol via dlsym() : %1")
			.argStrErrno().end();
	}

	//make a first try
	ssize_t res = libc_fwrite(ptr, size, nmemb, stream);

	//not ok => need to prefetch
	if (res == -1 && errno == EFAULT) {
		//get mapping and check it is backed to ummap-io
		ummapio::Mapping * mapping = ummapio::getGlobalhandler()->getMapping(buffer, false);

		//if not, it is not our problem, forward the error to the caller
		if (mapping == NULL)
			return -1;

		//get the max memory from the mapping policies
		size_t policy_max = mapping->getPolicyMaxMemory();

		//calc what to prefetch
		size_t fetch = std::min(size*nmemb, policy_max);
		fetch -= fetch % size;
		assumeArg(fetch > size, "Invalid fetch (%1) smaller than size (%2)")
			.arg(fetch)
			.arg(size)
			.end();

		//prefetch by reading one element per page
		ummap_touch(*mapping, buffer, fetch, true);

		//call again read
		size_t fetch_nmemb = fetch / size;
		res = libc_fwrite(ptr, size, fetch_nmemb, stream);

		//debug
		UMMAP_DEBUG_ARG("rw-wrapper", "Capture fwrite %1 => %2 => %3")
			.arg(buffer).arg(nmemb).arg(res).end();

		//recall
		if (res > 0 && res < nmemb) {
			//call again
			ssize_t res2 = fwrite(buffer + res*size, size, nmemb - res, stream);
			assumeArg(res2 >= 0, "Invalid status %1").arg(res2).end();
			res += res2;

			//debug
			UMMAP_DEBUG_ARG("rw-wrapper", "Capture fwrite recall %1 => %2 => %3")
				.arg(buffer).arg(nmemb).arg(res).end();
		}
	}
	return res;
}
