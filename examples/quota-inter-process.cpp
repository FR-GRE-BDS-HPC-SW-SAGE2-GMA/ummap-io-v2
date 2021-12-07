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
#include <sys/types.h>
#include <sys/wait.h>
//ummap-io
#include <ummap.h>

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//extract
	const char * driver_uri = "dummy://0";
	const char * policy_uri = "fifo://8KB";
	size_t size = 1024*1024;
	size_t segment_size = 4096;

	//set env for quota
	if (getenv("EXAMPLE_QUOTA") == NULL)
		setenv("EXAMPLE_QUOTA", "16KB", 1);

	//fork
	int pid = fork();
	int cur_pid = getpid();

	//init
	printf(" - (%d) Init...\n", cur_pid);
	ummap_init();

	//create quota
	ummap_quota_t * quota = ummap_quota_create_inter_proc_env("example-quota", "EXAMPLE_QUOTA", 0);
	printf(" - (%d) Quota: %s\n", cur_pid, getenv("EXAMPLE_QUOTA"));

	//map
	printf(" - (%d) Mapping...\n", cur_pid);
	ummap_driver_t * driver1 = ummap_driver_create_uri(driver_uri);
	ummap_policy_t * policy1 = ummap_policy_create_uri(policy_uri, true);
	ummap_quota_register_policy(quota, policy1);
	char * ptr1 = (char*)ummap(NULL, size, segment_size, 0, PROT_READ|PROT_WRITE, 0, driver1, policy1, "none");

	//access full mapping  1
	printf(" - (%d) Acces full mapping 1\n", cur_pid);
	memset(ptr1, 0, size);
	printf("   (%d) policy1 : %zu\n", cur_pid, ummap_policy_get_memory(policy1));

	//unmap
	printf(" - (%d) Unmapping...\n", cur_pid);
	umunmap(ptr1, 0);

	//close quota
	ummap_quota_destroy(quota);

	//fini
	printf(" - (%d) Finalize...\n", cur_pid);
	ummap_finalize();

	//wait
	if (pid != 0)
		waitpid(pid, NULL, 0);

	return EXIT_SUCCESS;
}
