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
		printf("%s {file_uri} {policy_uri} {size} {segment_size} [repeat]\n", argv[0]);
		return EXIT_FAILURE;
	}

	//extract
	const char * driver_uri = argv[1];
	const char * policy_uri = argv[2];
	size_t size = atol(argv[3]);
	size_t segment_size = atol(argv[4]);
	int repeat = 5;

	//optional
	if (argc == 6)
		repeat = atoi(argv[5]);

	//init
	printf(" - Init...\n");
	ummap_init();

	//map
	printf(" - Mapping...\n");
	ummap_driver_t * driver = ummap_driver_create_uri(driver_uri);
	ummap_policy_t * policy = ummmap_policy_create_uri(policy_uri, true);
	char * ptr = (char*)ummap(size, segment_size, 0, UMMAP_PROT_RW, driver, policy, "none");

	//5 memset
	int chunks = 20;
	int chunk_size = size / chunks;
	for (int i = 0 ; i < repeat ; i++) {
		for (int j = 0 ; j < chunks ; j++)
		{
			printf(" - Writing...\n");
			memset(ptr + j * chunk_size, i, chunk_size);
			printf(" - Waiting 1 seconds...\n");
			sleep(1);
		}

		//sync
		printf(" - Sync...\n");
		ummap_sync(ptr, 0);
		printf(" - Waiting 2 seconds...\n");
		sleep(2);
	}

	//unmap
	printf(" - Unmapping...\n");
	umunmap(ptr);

	//fini
	printf(" - Finalize...\n");
	ummap_finalize();

	return EXIT_SUCCESS;
}
