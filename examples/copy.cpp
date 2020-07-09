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
	if (argc < 6) {
		printf("%s {file_uri_in} {file_uri_out} {policy_uri} {size} {segment_size} [repeat]\n", argv[0]);
		return EXIT_FAILURE;
	}

	//extract
	const char * driver_uri_in = argv[1];
	const char * driver_uri_out = argv[2];
	const char * policy_uri = argv[3];
	size_t size = atol(argv[4]);
	size_t segment_size = atol(argv[5]);
	int repeat = 1;

	//optional
	if (argc == 7)
		repeat = atoi(argv[6]);

	//init
	printf(" - Init...\n");
	ummap_init();

	//map in
	printf(" - Mapping in...\n");
	ummap_driver_t * driver_in = ummap_driver_create_uri(driver_uri_in);
	ummap_policy_t * policy_in = ummap_policy_create_uri(policy_uri, true);
	char * in = (char*)ummap(size, segment_size, 0, UMMAP_PROT_READ, driver_in, policy_in, "none");

	//map in
	printf(" - Mapping in...\n");
	ummap_driver_t * driver_out = ummap_driver_create_uri(driver_uri_out);
	ummap_policy_t * policy_out = ummap_policy_create_uri(policy_uri, true);
	char * out = (char*)ummap(size, segment_size, 0, UMMAP_PROT_RW, driver_out, policy_out, "none");

	//write
	for (int i = 0 ; i < repeat ; i++) {
		printf(" - Writing...\n");
		memcpy(out, in, size);
	}
	
	//sync
	printf(" - Sync...\n");
	umsync(out, 0, 0);

	//unmap
	printf(" - Unmapping...\n");
	umunmap(in);
	umunmap(out);

	//fini
	printf(" - Finalize...\n");
	ummap_finalize();

	return EXIT_SUCCESS;
}
