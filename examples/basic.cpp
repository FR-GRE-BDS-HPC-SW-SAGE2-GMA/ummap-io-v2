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
//unix
#include <unistd.h>
//ummap-io
#include <ummap.h>

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//check
	if (argc < 5) {
		printf("%s {file_uri} {policy_uri} {size} {segment_size}\n", argv[0]);
		return EXIT_FAILURE;
	}

	//extract
	const char * driver_uri = argv[1];
	const char * policy_uri = argv[2];
	size_t size = atol(argv[3]);
	size_t segment_size = atol(argv[4]);

	//init
	printf(" - Init...\n");
	ummap_init();

	//map
	printf(" - Mapping...\n");
	ummap_driver_t * driver = ummap_driver_create_uri(driver_uri);
	ummap_policy_t * policy = ummap_policy_create_uri(policy_uri, true);
	char * ptr = (char*)ummap(size, segment_size, 0, UMMAP_PROT_RW, driver, policy, "none");

	//write
	printf(" - Writing...\n");
	memset(ptr, 10, size);
	printf(" - Waiting 1 seconds...\n");
	
	//sync
	printf(" - Sync...\n");
	ummap_sync(ptr, 0);

	//unmap
	printf(" - Unmapping...\n");
	umunmap(ptr);

	//fini
	printf(" - Finalize...\n");
	ummap_finalize();

	return EXIT_SUCCESS;
}
