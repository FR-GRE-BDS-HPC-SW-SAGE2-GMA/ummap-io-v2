/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

//mpicc -O0 main-policy-vs-swap.c -I${HOME}/test-rdma/usr2/include -lummap-io -L${HOME}/test-rdma/usr2/lib -o main-policy-vs-swap -fopenmp
//OMP_NUM_THREADS=8 ./main-policy-vs-swap 15 172 4

#include <stdbool.h>
#include <ummap/ummap.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <omp.h>
#include <mpi.h>

//#define SEGMENT_SIZE (2048UL*1024UL)
#define SEGMENT_SIZE (256UL*1024UL)
#define POLICY "fifo://32MB"
#define OP(x) x += i
#define STEP 4096

static inline double timespec_diff(struct timespec *a, struct timespec *b) {
	struct timespec result;
	result.tv_sec  = a->tv_sec  - b->tv_sec;
	result.tv_nsec = a->tv_nsec - b->tv_nsec;
	if (result.tv_nsec < 0) {
		--result.tv_sec;
		result.tv_nsec += 1000000000L;
	}
	return (double)result.tv_sec + (double)result.tv_nsec / (double)1e9;
}

void test_malloc(size_t size, size_t repeat)
{
	//get rank
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//start
	if (rank ==0)
		printf("Testing malloc...\n");

	//alloc
	char * ptr = malloc(size);
	assert(ptr != NULL);

	//wait wall
	MPI_Barrier(MPI_COMM_WORLD);

	//time
	struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);

	//loop
	size_t i, r;
	for (r = 0 ; r < repeat ; r++)
		//#pragma omp parallel for
		for (i = 0 ; i < size ; i+=STEP)
			OP(ptr[i]);

	//free
	free(ptr);

	//wait all
	MPI_Barrier(MPI_COMM_WORLD);

	//time
	clock_gettime(CLOCK_MONOTONIC, &stop);

	//print
	if (rank == 0) 
		printf("Testing malloc:   %0.03f seconds\n", timespec_diff(&stop, &start));
}

void test_mmap_file(size_t size, size_t repeat)
{
	//get rank
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//displau
	if (rank == 0)
		printf("Testing file map...\n");

	//open
	char fname[256];
	sprintf(fname, "/tmp/test-%d.raw", rank);
	FILE * fp = fopen(fname, "w+");
	ftruncate(fileno(fp), size);

	//mmap
	char * ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fileno(fp), 0);
	assert(ptr != MAP_FAILED);

	//wait all
	MPI_Barrier(MPI_COMM_WORLD);

        //time
        struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);

	//process
	size_t i, r;
	for (r = 0 ; r < repeat ; r++)
		//#pragma omp parallel for
		for (i = 0 ; i < size ; i+=STEP)
			OP(ptr[i]);

	//sync
	msync(ptr, size, MS_SYNC);

	//unmap
	munmap(ptr, size);

	//close
	fclose(fp);

	//wait all
	MPI_Barrier(MPI_COMM_WORLD);

	//time
	clock_gettime(CLOCK_MONOTONIC, &stop);

	//print
	if (rank == 0)
		printf("Testing file map: %0.03f seconds\n", timespec_diff(&stop, &start));
}

void test_ummap(size_t size, size_t repeat)
{
	//get rank
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//print
	if (rank == 0)
		printf("Testing ummap...\n");

	//map
	ummap_uri_set_variable_int("rank", rank);
	ummap_driver_t * driver = ummap_driver_create_uri("ioc://10:2{rank}");
	ummap_policy_t * policy = ummap_policy_create_uri(POLICY, true);
	char * ptr = ummap(NULL, size, SEGMENT_SIZE, 0, PROT_READ | PROT_WRITE, 0, driver, policy, "none");

	//wait all
	MPI_Barrier(MPI_COMM_WORLD);

	//start
	struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);

	//process
	size_t i, r;
	for (r = 0 ; r < repeat ; r++)
		//#pragma omp parallel for
		for (i = 0 ; i < size ; i+=STEP)
			OP(ptr[i]);

	//unmap
	umunmap(ptr, true);

	//wait all
	MPI_Barrier(MPI_COMM_WORLD);

	//time
	clock_gettime(CLOCK_MONOTONIC, &stop);

	//print
	if (rank == 0)
		printf("Testing ummap:    %0.03f seconds\n", timespec_diff(&stop, &start));
}

int main(int argc, char ** argv)
{
	//args
	if (argc < 4) {
		fprintf(stderr, "Usage: %s {seg_size} {baloon_size} {repeat}\n", argv[0]);
		return EXIT_FAILURE;
	}

	//mpi
	MPI_Init(&argc, &argv);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	//info
	if (rank == 0) {
		printf("============= Parameters ==============\n");
		printf("Size    : %0.02f GB\n", atof(argv[1]));
		printf("Balloon : %0.02f GB\n", atof(argv[2]));
		printf("Segment : %lu KB\n", SEGMENT_SIZE/1024UL);
		printf("Policy  : %s\n", POLICY);
		printf("Repeat  : %lu\n", atol(argv[3]));
		printf("Ranks   : %d\n", world_size);
	}

	//extract args
	size_t size = atof(argv[1]) * 1024.0 * 1024.0 * 1024.0;
	size_t balloon = atof(argv[2]) * 1024.0 * 1024.0 * 1024.0;
	size_t repeat = atol(argv[3]);

	//divide
	size /= world_size;
	size -= size % (SEGMENT_SIZE);
	balloon /= world_size;

	//init ummap
	ummap_init();
	ummap_config_ioc_init_options("10.1.3.85", "8556");

	//test without ballon
	if (rank == 0) printf("\n============= No balloon ==============\n");
	test_ummap(size, repeat);
	test_mmap_file(size, repeat);
	test_malloc(size, repeat);

	//ballon
	if (rank == 0) printf("\n============ With balloon =============\n");
	if (rank == 0) printf("Create balloon...\n");
	MPI_Barrier(MPI_COMM_WORLD);
	char * ptr = malloc(balloon);
	size_t i;
	for (i = 0 ; i < balloon ; i+= 4096)
		ptr[i] = 1;	
	mlock(ptr, size);
	MPI_Barrier(MPI_COMM_WORLD);

	//test with balloon
	test_ummap(size, repeat);
	test_mmap_file(size, repeat);
	test_malloc(size, repeat);
	
	//free balloon
	if (rank == 0) free(ptr);

	//fini
	ummap_finalize();

	//mpi
	MPI_Finalize();
	
	//ok
	return EXIT_SUCCESS;
}

