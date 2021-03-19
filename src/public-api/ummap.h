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

/********************  FLAGS  **********************/
/** Default value for the ummap flags. **/
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
typedef struct ioc_client_s ioc_client_t;

/****************  C DRIVER STRUCT  ******************/
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

/*********************  INIT  ***********************/
/**
 * Initialization of the ummap-io library, mostly to setup the signal handling for SIG_SEGV.
**/
void ummap_init(void);
/**
 * Finalize the ummap-io library, mostly to remove the signal handler for SIG_SEGV.
**/
void ummap_finalize(void);

/*******************  MAPPINGS  *********************/
/**
 * Establish a new memory mapping.
 * @param addr Define the memory address where to place the mapping. NULL for default it can be forced
 * by using flag UMMAP_MAP_FIXED.
 * @param size Define the size of the mapping. If can be a non multiple of segment size. In this case, the mapping itself
 * will be sized to the next multiple. For read/write operations it will ignore this extra sub-segment.
 * @param segmentSize Equivalent of the page size for a standard mmap, it define the granularity of the IO operations.
 * This size must be a multiple of the OS page size (4K).
 * @param storageOffset Offset to apply on the storage of reach the data to be mapped. It does not have to be aligned on
 * page size.
 * @param protection Define the access protection to assign to this mapping. It uses the flags from mmap so you can
 * use the given flags and 'or' them: PROT_READ, PROT_WRIT, PROT_EXEC.
 * @param flags Flags to enable of disable some behaviors of ummap-io. Currently valid flags are : UMMAP_NO_FIRST_READ, 
 * UMMAP_THREAD_UNSAFE. Go in their respective documentation to get more information on them. You can also use UMMAP_FIXED
 * to force the targetted address to establish the mapping.
 * @param driver Pointer to the given driver. If UMMAP_DRVIER_NO_AUTO_DELETE if enabled the destruction of the driver is you 
 * own responsability, otherwise it will be destroyed automatically.
 * @param localPolicy Define the local policy to be used.
 * @param globalPolicy Define the global policy to be used and shared between multiple mappings.
**/
void * ummap(void * addr, size_t size, size_t segment_size, size_t storage_offset, int protection, int flags, ummap_driver_t * driver, ummap_policy_t * local_policy, const char * policy_group);
/**
 * Unmap the given mapping if found.
 * @param ptr Base address of the mapping or an address inside the mapping.
 * @param sync If true make a synchronization by flusing data before unmapping or not if false.
**/
int umunmap(void * ptr, bool sync);
/**
 * Apply a sync operation to flush data to the storage.
 * @param ptr Base address of the mapping or an address inside the mapping.
 * @param size Size of the range to sync of 0 for all.
 * @param evict If true evict the synced segments after making the sync operation.
**/
void umsync(void * ptr, size_t size, bool evict);

/********************  SETUP  ***********************/
/**
 * Mark all pages an not needing a read operation on first access.
 * @param ptr Base address of the mapping or an address inside the mapping.
**/
void ummap_skip_first_read(void * ptr);

/********************  DRIVERS  *********************/
/**
 * Build a new driver from the given URI.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param uri Define the URI to apply, examples :
 *    - file://tmp.raw
 *    - ioc://10:22
 *    - mero://10:22
 *    - dummy://0
 * @return Pointer to the created driver.
**/
ummap_driver_t * ummap_driver_create_uri(const char * uri);
/**
 * Build a new driver to handle files via a filename.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param file_path Path of the file to map.
 * @param mode Open mode like for fopen().
 * @return Pointer to the created driver.
**/
ummap_driver_t * ummap_driver_create_fopen(const char * file_path, const char * mode);
/**
 * Build a new DAX driver for files. In other words it make a direct mmap to the file.
 * It can be used to directly map NVDIMM if using a DAX mount point.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param file_path Path of the file to map.
 * @param mode Open mode like for fopen().
 * @param allox_not_aligned Allow not aligned size and offets it will emulate by padding the size and the address.
**/
ummap_driver_t * ummap_driver_create_dax_fopen(const char * file_path, const char * mode, bool allow_not_aligned);
/**
 * Build a new driver to handle files via a file descriptor.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param fd File descriptor to be used to access to the file. It will make a dup()
 * so you can safely close the file descriptor on your side.
 * @return Pointer to the created driver.
**/
ummap_driver_t * ummap_driver_create_fd(int fd);
/**
 * Build a new DAX driver for files. In other words it make a direct mmap to the file.
 * It can be used to directly map NVDIMM if using a DAX mount point.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param fd File descriptor to be used to access to the file. It will make a dup()
 * so you can safely close the file descriptor on your side.
 * @param allox_not_aligned Allow not aligned size and offets it will emulate by padding the size and the address.
**/
ummap_driver_t * ummap_driver_create_dax_fd(int fd, bool allow_not_aligned);
/**
 * Create a mapping attached to a second memory space and copy data from/to this memory space.
 * This is more for testing ummap performances.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param size Size of the memory space.
**/
ummap_driver_t * ummap_driver_create_memory(size_t size);
/**
 * Create a dummy driver just copying a fixed value to the mapped space. This is more for testing
 * ummap performances.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param value The value to be copied via memset to the memory mapping.
**/
ummap_driver_t * ummap_driver_create_dummy(char value);
/**
 * Instanciate a C driver by linking the function implementation structure with a data pointer.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param driver Pointer to the structure containing the driver implementation via function pointers.
 * The struct will be copied so the lifecycle of the struct pointed by this parameter is the responsibility
 * of the caller.
 * @param driver_data A raw pointer poiting the state to be transmitted to all functions on call.
**/
ummap_driver_t * ummap_driver_create_c(const ummap_c_driver_t * driver, void * driver_data);
/**
 * Create a driver to use the IO-Catcher making a cache possibly on NVDIMM between the client
 * and Mero.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param client Point the IOC client.
 * @param high Define the high part of the object ID to access.
 * @param low Define the low part of the object ID to access.
 * @param create Define if we need to create the object prior to accesses.
**/
ummap_driver_t * ummap_driver_create_ioc(ioc_client_t * client, int64_t high, int64_t low, bool create);
/**
 * Create a driver to map Mero object via the Clovis API.
 * The driver is created with auto cleanup to be destroyed on umunmap().
 * @param high Define the high part of the object ID to access.
 * @param low Define the low part of the object ID to access.
 * @param create Define if we need to create the object prior to accesses.
**/
ummap_driver_t * ummap_driver_create_clovis(int64_t hight, int64_t low, bool create);

/**
 * Destroy the driver. Not if you do not call ummap_driver_set_autoclean() ummap
 * will do it automatically when unmapping the related segment.
 * @param driver Pointer to the driver to destroy.
**/
void ummap_driver_destroy(ummap_driver_t * driver);
/**
 * Permit to enable or disable the autoclean feature. If enabled ummap will automatically
 * destroy the driver while calling umunmap() on the mapping.
 * @param driver Define the driver to inpact.
 * @param autoclean Boolean to enable or disable the autoclean feature.
**/
void ummap_driver_set_autoclean(ummap_driver_t * driver, bool autoclean);

/****************  POLICY GROUPS  *******************/
/**
 * Register a policy under the given policy group name.
 * @param name Name of the policy group.
 * @param policy Pointer to the policy to apply.
**/
void ummap_policy_group_register(const char * name, ummap_policy_t * policy);
/**
 * Destroy a policy group by its name.
 * @param name Name of the policy group to destroy.
**/
void ummap_policy_group_destroy(const char * name);

/*******************  POLICIES  *********************/
/**
 * Create a policy by giving an URI.
 * @param uri Define the URI of the policy to instanciate. Can be:
 *   - none
 *   - fifo://1MB
 *   - fifo-window://2MB?window=1MB
 *   - lifo://1MB
 * @param local Define if it is a local policy to avoid locks or a policy group shared
 * between multiple mappings.
**/
ummap_policy_t * ummap_policy_create_uri(const char * uri, bool local);
/**
 * Build a fifo policy handler.
 * @param max_size Define the maximum memory allowed by this policy.
 * @param local Define if it is a local policy to avoid locks or a policy group shared
 * between multiple mappings.
**/
ummap_policy_t * ummap_policy_create_fifo(size_t max_size, bool local);
/**
 * Build a fifo-window policy handler.
 * @param max_size Define the maximum memory allowed by this policy.
 * @param local Define if it is a local policy to avoid locks or a policy group shared
 * between multiple mappings.
**/
ummap_policy_t * ummap_policy_create_fifo_window(size_t max_size, size_t window_size, bool local);
/**
 * Build a lifo policy handler.
 * @param max_size Define the maximum memory allowed by this policy.
 * @param local Define if it is a local policy to avoid locks or a policy group shared
 * between multiple mappings.
**/
ummap_policy_t * ummap_policy_create_lifo(size_t max_size, bool local);
/**
 * Destroy the given policy. In practice you do not have to do it by hand as it is
 * done by the umunmap() call.
 * @param policy Pointer to the policy to destroy.
**/
void ummap_policy_destroy(ummap_policy_t * policy);

/*****************  URI VARIABLES  ******************/
/**
 * Set a variable value as string for the URI system.
 * @param name Name of the variable.
 * @param value Value as string.
**/
void ummap_uri_set_variable(const char * name, const char * value);
/**
 * Set a variable value as string for the URI system.
 * @param name Name of the variable.
 * @param value Value as integer.
**/
void ummap_uri_set_variable_int(const char * name, int value);
/**
 * Set a variable value as string for the URI system.
 * @param name Name of the variable.
 * @param value Value as size_t.
**/
void ummap_uri_set_variable_size_t(const char * name, size_t value);

/**************  EXTRA DRIVER CONFIGS  **************/
/**
 * Init clovis options for the URI system.
 * @param ressource_file Point the Clovis ressource file to access the server.
 * @param index Index of the configuration in the ressource file.
**/
void ummap_config_clovis_init_options(const char * ressource_file, int index);
/**
 * Init the IO-Catcher information to access the server for the URI system.
 * @param server IP of the server.
 * @param port Port of the server.
**/
void ummap_config_ioc_init_options(const char * server, const char * port);

/*****************  COW OPERATIONS  *****************/
/**
 * Replace the underlying file with a copy version.
 * @param addr An adress in the mapping to impact.
 * @param file_path Path of the file to map.
 * @param mode Open mode like for fopen().
 * @param allow_exist Allow to COW on an object which already exist.
 * @return 0 on success, negative value in case of error.
 * @warning CAUTION, this does not support multiple mappings sharing the same driver.
**/
int ummap_cow_fopen(void * addr, const char * file_path, const char * mode, bool allow_exist);
/**
 * Replace the underlying file with a copy version.
 * @param addr An adress in the mapping to impact.
 * @param fd File descriptor to be used to access to the file. It will make a dup()
 * so you can safely close the file descriptor on your side.
 * @param allow_exist Allow to COW on an object which already exist.
 * @return 0 on success, negative value in case of error.
 * @warning CAUTION, this does not support multiple mappings sharing the same driver.
**/
int ummap_cow_fd(void * addr, int fd, bool allow_exist);
/**
 * Replace the underlying object with a copy on write version.
 * @param addr An adress in the mapping to impact.
 * @param high The high part of the new object ID.
 * @param low The low pard of the new object ID.
 * @param allow_exist Allow to COW on an object which already exist.
 * @return 0 on success, negative value in case of error.
 * @warning CAUTION, this does not support multiple mappings sharing the same driver.
**/
int ummap_cow_ioc(void * addr, int64_t high, int64_t low, bool allow_exist);
/**
 * Replace the underlying object with a copy on write version.
 * @param addr An adress in the mapping to impact.
 * @param uri The new URI to apply on the driver.
 * @param allow_exist Allow to COW on an object which already exist.
 * @return 0 on success, negative value in case of error.
 * @warning CAUTION, this does not support multiple mappings sharing the same driver.
**/
int ummap_cow_uri(void * addr, const char * uri, bool allow_exist);

/****************  SWITCH OPERATIONS  ***************/
/**
 * Change the object ID of the given IOC driver to another object.
 * @param addr An adress in the mapping to impact.
 * @param high The high part of the next object ID.
 * @param low The low part of the next object ID.
 * @param drop_clean If true drop the clean pages so they will be reloaded from the object.
 * @return 0 on success, negative value on error.
 * @warning CAUTION, this does not support multiple mappings sharing the same driver.
**/
int ummap_switch_ioc(void * addr, int64_t high, int64_t low, bool drop_clean);
/**
 * Replace the underlying object with a copy on write version.
 * @param addr An adress in the mapping to impact.
 * @param uri The new URI to apply on the driver.
 * @param drop_clean If true drop the clean pages so they will be reloaded from the object.
 * @return 0 on success, negative value in case of error.
 * @warning CAUTION, this does not support multiple mappings sharing the same driver.
**/
int ummap_switch_uri(void * addr, const char * uri, bool drop_clean);

#ifdef __cplusplus
}
#endif

#endif //UMMAP_H
