/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_H
#define UMMAP_H

/********************  HEADERS  *********************/
//std
#include <stdlib.h>
#include <stdbool.h>
//unix
#include <sys/mman.h>

#ifdef __cplusplus
extern "C" {
#endif

/********************  CONSTS  **********************/
/** Default value for the ummap flags **/
#define UMMAP_DEFAULT 0
/** 
 * Do not load data from the storage for the first read access.
 * This is usefull to create a new file/object and directly write
 * the data in withoug making a first read operation.
**/
#define UMMAP_NO_FIRST_READ 1
/**
 * Disable thread safety by not using temporary mprotect(PROT_READ) while
 * flushing data and using mremap() semantic to make load & insert opertation
 * atomic.
**/
#define UMMAP_THREAD_UNSAFE 2
/** Force the mapping address to the given hint like MAP_FIXED for mmap(). **/
#define UMMAP_FIXED 4

/*********************  TYPES  **********************/
/** Hidden struct used to point ummap C++ policies. **/
typedef struct ummap_policy_s ummap_policy_t;
/** Hidden struct used to point the mmap C++ drivers. **/
typedef struct ummap_driver_s ummap_driver_t;
/** Hidden struct used to point the ioc client when using ioc. **/
struct ioc_client_t;

/*******************  FUNCTION  *********************/
/**
 * Interface to implement a C driver for ummap by providing the required
 * function pointers to the implementation.
**/
typedef struct ummap_c_driver_s {
	/**
	 * Apply write operation to dump the memory content into the related IO storage.
	 * @param driver_data The extra data attached to the driver to store states.
	 * @param buffer Buffer pointer containing the data to write.
	 * @param size Size of the data to write.
	 * @param offset Offset in the storage element.
	 * @return The writted size.
	**/
	ssize_t (*pwrite)(void * driver_data, const void * buffer, size_t size, size_t offset);
	/**
	 * Apply read operation to load the content into memory from the related IO storage.
	 * @param driver_data The extra data attached to the driver to store states.
	 * @param buffer Buffer pointer in which to load the data.
	 * @param size Size of the data to read.
	 * @param offset Offset in the storage element.
	 * @return The read size.
	**/
	ssize_t (*pread)(void * driver_data, void * buffer, size_t size, size_t offset);
	/**
	 * Apply a sync operation on the given interval.
	 * @param driver_data The extra data attached to the driver to store states.
	 * @param ptr The pointer of the segment to sync (without accounting offset which need to be added).
	 * @param offset From where to start the sync operation.
	 * @param size Size of the segment to sync.
	**/
	void (*sync)(void * driver_data, void *ptr, size_t offset, size_t size);
	/**
	 * Let the driver making the memory mapping. This is to be used
	 * by direct access modes
	 * @param driver_data The extra data attached to the driver to store states.
	 * @param addr Define the optional address where to map.
	 * @param size Define the mapping size.
	 * @param offset Define the offset in file.
	 * @param read Accept read accesses
	 * @param write Accept write accesses
	 * @param mapFixed Force the mapping address to addr.
	 * @return NULL if the caller has to do the mapping, an addresse otherwise.
	**/
	void * (*direct_mmap)(void * addr, void * driver_data, size_t size, size_t offset, bool read, bool write, bool exec, bool mapFixed);
	/**
	 * Let the driver maling the memory unmaping.
	 * @param driver_data The extra data attached to the driver to store states.
	 * @param base Base addresse of the mapping.
	 * @param size Size of the mapping.
	 * @param offset Define the offset in file.
	 * @return Return true if it has unmapped, false if it need to be done by the caller.
	**/
	bool (*direct_munmap)(void * driver_data, void * base, size_t size, size_t offset);
	/**
	 * When directly mapping do not make the ummap internal state tracking but direct msync
	 * @param driver_data The extra data attached to the driver to store states.
	 * @param base Base addresse of the mapping.
	 * @param size Size of the mapping.
	 * @param offset Define the offset in file.
	**/
	bool (*direct_msync)(void * driver_data, void * base, size_t size, size_t offset);
	/**
	 * Function used to cleanup the driver state before exit.
	 * @param driver_data The extra data attached to the driver to store states.
	**/
	void (*finalize)(void * driver_data);
} ummap_c_driver_t;

/*******************  FUNCTION  *********************/
//global init/destroy
void ummap_init(void);
void ummap_finalize(void);

/*******************  FUNCTION  *********************/
//ummap
void * ummap(void * addr, size_t size, size_t segment_size, size_t storage_offset, int protection, int flags, ummap_driver_t * driver, ummap_policy_t * local_policy, const char * policy_group);
int umunmap(void * ptr, int sync);
void umsync(void * ptr, size_t size, bool evict);

/*******************  FUNCTION  *********************/
//setup
void ummap_skip_first_read(void * ptr);

/*******************  FUNCTION  *********************/
//drivers
ummap_driver_t * ummap_driver_create_uri(const char * uri);
ummap_driver_t * ummap_driver_create_fopen(const char * file_path, const char * mode);
ummap_driver_t * ummap_driver_create_dax_fopen(const char * file_path, const char * mode, bool allow_not_aligned);
ummap_driver_t * ummap_driver_create_fd(int fd);
ummap_driver_t * ummap_driver_create_dax_fd(int fd, bool allow_not_aligned);
ummap_driver_t * ummap_driver_create_memory(size_t size);
ummap_driver_t * ummap_driver_create_dummy(char value);
ummap_driver_t * ummap_driver_create_c(const ummap_c_driver_t * driver, void * driver_data);
ummap_driver_t * ummap_driver_create_ioc(struct ioc_client_t * client, int64_t high, int64_t low, bool create);
ummap_driver_t * ummap_driver_create_clovis(int64_t hight, int64_t low, bool create);

void ummap_driver_destroy(ummap_driver_t * driver);
void ummap_driver_set_autoclean(ummap_driver_t * driver, bool autoclean);

/*******************  FUNCTION  *********************/
//policy groups
void ummap_policy_group_register(const char * name, ummap_policy_t * policy);
void ummap_policy_group_destroy(const char * name);

/*******************  FUNCTION  *********************/
//policies
ummap_policy_t * ummap_policy_create_uri(const char * uri, bool local);
ummap_policy_t * ummap_policy_create_fifo(size_t max_size, bool local);
ummap_policy_t * ummap_policy_create_lifo(size_t max_size, bool local);
void ummap_policy_destroy(ummap_policy_t * policy);

/*******************  FUNCTION  *********************/
//uri variables
void ummap_uri_set_variable(const char * name, const char * value);
void ummap_uri_set_variable_int(const char * name, int value);
void ummap_uri_set_variable_size_t(const char * name, size_t value);

/*******************  FUNCTION  *********************/
//extra driver configs
void ummap_config_clovis_init_options(const char * ressource_file, int index);
void ummap_config_ioc_init_options(const char * server, const char * port);

#ifdef __cplusplus
}
#endif

#endif //UMMAP_H
