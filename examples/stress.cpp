/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <cstdio>
#include <cstring>
//openmp
#include <omp.h>
//unix
#include <unistd.h>
//ummap-io
#include <ummap.h>

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//check
	if (argc < 2) {
		printf("%s {file_uri} [repeat]\n", argv[0]);
		return EXIT_FAILURE;
	}

	//extract
	const char * driver_uri = argv[1];
	size_t size = 16*1024*1024;
	size_t segment_size = 4096;
	int repeat = 100;

	//optional
	if (argc == 3)
		repeat = atoi(argv[2]);

	//init
	printf(" - Init...\n");
	ummap_init();

	//create global policy
	int threads = omp_get_max_threads();
	ummap_policy_t * global_policy = ummap_policy_create_fifo(threads * 4096 * 2, false);
	ummap_policy_group_register("global", global_policy);

	//threads
	#pragma omp parallel
	{
		//map
		printf(" - Mapping...\n");
		ummap_driver_t * driver = ummap_driver_create_uri(driver_uri);
		ummap_policy_t * policy = ummap_policy_create_fifo(4096 * 4, false);
		char * ptr = (char*)ummap(size, segment_size, 0, UMMAP_PROT_RW, driver, policy, "global");

		//write
		for (int i = 0 ; i < repeat ; i++) {
			printf(" - Writing [%d]...\n", i);
			memset(ptr, 10, size);
		}

		//sync
		printf(" - Sync...\n");
		umsync(ptr, 0, 0);

		//unmap
		printf(" - Unmapping...\n");
		umunmap(ptr);
	}

	//fini
	printf(" - Finalize...\n");
	ummap_finalize();

	return EXIT_SUCCESS;
}
