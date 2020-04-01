/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_H
#define UMMAP_H

/********************  HEADERS  *********************/
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************  TYPES  **********************/
typedef struct ummap_policy_s ummap_policy_t;
typedef struct ummap_driver_s ummap_driver_t;

/*********************  TYPES  **********************/
typedef enum ummap_mapping_prot_s
{
	UMMAP_PROT_NONE = 0,
	UMMAP_PROT_READ = 1,
	UMMAP_PROT_WRITE = 2,
	UMMAP_PROT_RW = 3,
} ummap_mapping_prot_t;

/*******************  FUNCTION  *********************/
//global init/destroy
void ummap_init(void);
void ummap_finalize(void);

/*******************  FUNCTION  *********************/
//ummap
void * ummap(size_t size, size_t segment_size, size_t storage_offset, ummap_mapping_prot_t protection, ummap_driver_t * driver, ummap_policy_t * local_policy, const char * policy_group);
int umunmap(void * ptr);

/*******************  FUNCTION  *********************/
//drivers
ummap_driver_t * ummap_driver_create_fname(const char * file_path);
ummap_driver_t * ummap_driver_create_fd(int fd);
ummap_driver_t * ummap_driver_create_memory(size_t size);
ummap_driver_t * ummap_driver_create_dummy(char value);
void ummap_driver_destroy(ummap_driver_t * driver);
void ummap_driver_set_autoclean(ummap_driver_t * driver, bool autoclean);

/*******************  FUNCTION  *********************/
//policy groups
void ummap_policy_group_register(const char * name, ummap_policy_t * policy);
void ummap_policy_group_destroy(const char * name);

/*******************  FUNCTION  *********************/
//policies
ummap_policy_t * umamp_policy_create_fifo(size_t max_size);

#ifdef __cplusplus
}
#endif

#endif //UMMAP_H