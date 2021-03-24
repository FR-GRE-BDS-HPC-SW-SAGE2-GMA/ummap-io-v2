/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
//unix
#include <unistd.h>
//compiler
#include <x86intrin.h>
//ummap-io
#include <ummap.h>

/*********************  CONSTS  *********************/
//consts
#define SEGMENTS 1024
#define SEGMENT_SIZE (1024*1024)
#define SIZE (SEGMENTS * SEGMENT_SIZE)

/*******************  FUNCTION  *********************/
void fill_segment(char * ptr, size_t seg_id, size_t cnt, int value)
{
	memset(ptr + SEGMENT_SIZE * seg_id, value, SEGMENT_SIZE * cnt);
}

/*******************  FUNCTION  *********************/
void check_segment(char * ptr, size_t seg_id, size_t cnt, int value)
{
	size_t i;
	for (i = 0 ; i < SEGMENT_SIZE * cnt ; i++) {
		const size_t index = seg_id * SEGMENT_SIZE + i;
		const int cur = ptr[index];
		if (cur != value) {
			printf("Invalid value in memory in segment %zu : %d != %d !\n", seg_id, cur, value);
			abort();
		}
	}
}

/*******************  FUNCTION  *********************/
#define MEASURE(name, code) do {\
		uint64_t start = _rdtsc(); \
		code; \
		uint64_t stop = _rdtsc(); \
		printf("     > %-30s : %0.2f MCycles\n", name, (float)(stop-start)/1000.0/1000.0); \
	} while(0)

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//check
	if (argc < 5) {
		printf("%s {server_addr} {orig_uri} {copy_uri} {switch_uri}\n", argv[0]);
		return EXIT_FAILURE;
	}

	//extract
	const char * server_addr = argv[1];
	const char * orig_uri = argv[2];
	const char * copy_uri = argv[3];
	const char * switch_uri = argv[4];

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
	MEASURE("memset", memset(ptr, 10, SIZE));
	MEASURE("umsync", umsync(ptr, 0,false));
	MEASURE("uunmap", umunmap(ptr, true));

	////////////////////////// STEP 2 : map & check & write touch //////////////////////
	//remap to access just a part
	printf (" - Remap to write just a part to make it dirty...\n");
	driver = ummap_driver_create_uri(orig_uri);
	ptr = (char*)ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ | PROT_WRITE, 0, driver, NULL, "none");

	//access part
	MEASURE("fill_segment 1/3->2/3", fill_segment(ptr, SEGMENTS / 3, SEGMENTS / 3, 20));
	MEASURE("fill_segment 1/3->2/3", fill_segment(ptr, SEGMENTS / 3, SEGMENTS / 3, 20));
	check_segment(ptr, 5, 1, 10);

	///////////////////////// STEP 3: make manual copy /////////////////////////////////
	printf (" - Make a manual copy...\n");
	driver = ummap_driver_create_uri(copy_uri);
	char * ptr_copy = (char*)ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ | PROT_WRITE, 0, driver, NULL, "none");

	//copy
	MEASURE("memcpy 1/3->1/3", memcpy(ptr_copy, ptr + SIZE / 3, SIZE / 3));
	MEASURE("fill_segment 1/3->2/3", fill_segment(ptr_copy, SEGMENTS / 3, SEGMENTS / 3, 30));
	MEASURE("fill_segment 1/3->2/3", fill_segment(ptr_copy, SEGMENTS / 3, SEGMENTS / 3, 30));
	MEASURE("umsync", umsync(ptr_copy, 0,false));
	MEASURE("uunmap", umunmap(ptr_copy, true));

	//////////////////////// STEP 4: make a switch and flush //////////////////////////
	printf (" - Make a SWITCH copy...\n");

	//make switch
	MEASURE("make switch", ummap_switch_uri(ptr, switch_uri, true));

	//copy
	MEASURE("fill_segment 1/3->2/3", fill_segment(ptr, SEGMENTS / 3, SEGMENTS / 3, 30));
	MEASURE("fill_segment 1/3->2/3", fill_segment(ptr, SEGMENTS / 3, SEGMENTS / 3, 30));
	MEASURE("umsync", umsync(ptr, 0,false));
	MEASURE("uunmap", umunmap(ptr, true));

	//fini
	printf(" - Finalize...\n");
	ummap_finalize();

	//ok
	printf(">>>>>>>>>>>>> ALL OK <<<<<<<<<<<\n");

	return EXIT_SUCCESS;
}
