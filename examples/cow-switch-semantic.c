/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//unix
#include <unistd.h>
//ummap-io
#include <ummap.h>

/*********************  CONSTS  *********************/
//consts
#define SEGMENTS 16
#define SEGMENT_SIZE (1024*1024)
#define SIZE (SEGMENTS * SEGMENT_SIZE)

/*******************  FUNCTION  *********************/
void fill_segment(char * ptr, size_t seg_id, int value)
{
	memset(ptr + SEGMENT_SIZE * seg_id, value, SEGMENT_SIZE);
}

/*******************  FUNCTION  *********************/
void check_segment(char * ptr, size_t seg_id, int value)
{
	size_t i;
	for (i = 0 ; i < SEGMENT_SIZE ; i++) {
		const size_t index = seg_id * SEGMENT_SIZE + i;
		const int cur = ptr[index];
		if (cur != value) {
			printf("Invalid value in memory in segment %zu : %d != %d !\n", seg_id, cur, value);
			abort();
		}
	}
}

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//check
	if (argc < 4) {
		printf("%s {server_addr} {orig_uri} {cow_uri}\n", argv[0]);
		return EXIT_FAILURE;
	}

	//extract
	const char * server_addr = argv[1];
	const char * orig_uri = argv[2];
	const char * cow_uri = argv[3];
	const char * switch_uri = orig_uri;

	//init
	printf(" - Init...\n");
	ummap_init();

	//setup config
	ummap_config_ioc_init_options(server_addr, "8556");

	////////////////////////////// STEP 1 : map & fill ////////////////////////////////
	//map orig
	printf(" - Mapping orig step 1...\n");
	ummap_driver_t * driver = ummap_driver_create_uri(orig_uri);
	char * ptr = (char*)ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ | PROT_WRITE, UMMAP_NO_FIRST_READ, driver, NULL, "none");

	//memset & unamp sync
	memset(ptr, 10, SIZE);
	umunmap(ptr, true);

	////////////////////////// STEP 2 : map & check & write touch //////////////////////
	//remap to access just a part
	printf (" - Remap to access just a part...\n");
	driver = ummap_driver_create_uri(orig_uri);
	ptr = (char*)ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ | PROT_WRITE, 0, driver, NULL, "none");

	//access part
	fill_segment(ptr, 3, 20);
	check_segment(ptr, 5, 10);

	/////////////////////////////// STEP 3 : cow & check ///////////////////////////////
	//cow
	printf(" - COW & check...\n");
	ummap_cow_uri(ptr, cow_uri, true);
	check_segment(ptr, 3, 20);
	check_segment(ptr, 4, 10);
	check_segment(ptr, 5, 10);

	//unmap & flush
	umunmap(ptr, true);

	/////////////////////////////// STEP 4 : remap cow & check /////////////////////////
	//mmap & check
	printf (" - Remap to access just a part...\n");
	driver = ummap_driver_create_uri(cow_uri);
	ptr = (char*)ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ | PROT_WRITE, 0, driver, NULL, "none");

	//check
	check_segment(ptr, 1, 10);
	check_segment(ptr, 3, 20);
	check_segment(ptr, 4, 10);
	check_segment(ptr, 5, 10);

	////////////////////////////// STEP 5 : write touch & switch ////////////////////////
	printf(" - Write touch & switch back to orig & check again...\n");
	fill_segment(ptr, 8, 30);
	ummap_switch_uri(ptr, switch_uri, true);
	check_segment(ptr, 1, 10);
	check_segment(ptr, 3, 10);
	check_segment(ptr, 4, 10);
	check_segment(ptr, 5, 10);
	check_segment(ptr, 8, 30);
	umunmap(ptr, true);

	////////////////////////////// STEP 6 : remap orig & final check /////////////////////
	driver = ummap_driver_create_uri(switch_uri);
	ptr = (char*)ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ | PROT_WRITE, 0, driver, NULL, "none");
	check_segment(ptr, 1, 10);
	check_segment(ptr, 3, 10);
	check_segment(ptr, 4, 10);
	check_segment(ptr, 5, 10);
	check_segment(ptr, 8, 30);
	umunmap(ptr, false);

	//fini
	printf(" - Finalize...\n");
	ummap_finalize();

	//ok
	printf(">>>>>>>>>>>>> ALL OK <<<<<<<<<<<\n");

	return EXIT_SUCCESS;
}
