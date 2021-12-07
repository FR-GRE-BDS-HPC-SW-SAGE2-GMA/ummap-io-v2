/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <cstdio>
#include <cstring>
//unix
#include <unistd.h>
//ummap-io
#include <ummap.h>

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//extract
	const char * driver_uri = "dummy://0";
	const char * policy_uri = "fifo://8KB";
	size_t size = 16*4096;
	size_t segment_size = 4096;

	//init
	printf(" - Init...\n");
	ummap_init();

	//map
	printf(" - Mapping 1...\n");
	ummap_driver_t * driver1 = ummap_driver_create_uri(driver_uri);
	ummap_policy_t * policy1 = ummap_policy_create_uri(policy_uri, true);
	char * ptr1 = (char*)ummap(NULL, size, segment_size, 0, PROT_READ|PROT_WRITE, 0, driver1, policy1, "none");

	//map
	printf(" - Mapping 2...\n");
	ummap_driver_t * driver2 = ummap_driver_create_uri(driver_uri);
	ummap_policy_t * policy2 = ummap_policy_create_uri(policy_uri, true);
	char * ptr2 = (char*)ummap(NULL, size, segment_size, 0, PROT_READ|PROT_WRITE, 0, driver2, policy2, "none");

	//quota
	ummap_quota_t * quota = ummap_quota_create_local(4*4096);
	ummap_quota_register_policy(quota, policy1);
	ummap_quota_register_policy(quota, policy2);

	//access full mapping  1
	printf(" - Acces full mapping 1\n");
	memset(ptr1, 0, size);
	printf("   policy1 : %zu\n", ummap_policy_get_memory(policy1));
	printf("   policy2 : %zu\n", ummap_policy_get_memory(policy2));

	//access full mapping  1
	printf(" - Acces full mapping 2\n");
	memset(ptr2, 0, size);
	printf("   policy1 : %zu\n", ummap_policy_get_memory(policy1));
	printf("   policy2 : %zu\n", ummap_policy_get_memory(policy2));

	//unmap
	printf(" - Unmapping...\n");
	umunmap(ptr1, 0);
	umunmap(ptr2, 0);

	//fini
	printf(" - Finalize...\n");
	ummap_finalize();

	return EXIT_SUCCESS;
}
