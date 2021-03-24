/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include <sys/mman.h>
#include "../ummap.h"

/*********************  CLASS  **********************/
class TestPublicAPI : public testing::Test
{
	public:
		virtual void SetUp(void);
		virtual void TearDown(void);
};

/*******************  FUNCTION  *********************/
void TestPublicAPI::SetUp(void)
{
	ummap_init();
}

/*******************  FUNCTION  *********************/
void TestPublicAPI::TearDown(void)
{
	ummap_finalize();
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, init_finalize)
{
	//just setup/teardown
}


/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, nested_init)
{
	ummap_init();
	ummap_finalize();
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_unmap_simple)
{
	//map 2
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_dummy(16), NULL, "none");
	memset(ptr1, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1, 0);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_unmap_multi)
{
	//map 2
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_dummy(16), NULL, "none");
	void * ptr2 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_dummy(16), NULL, "none");

	//touch
	memset(ptr1, 10, 8*4096);
	memset(ptr2, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1, 0);
	umunmap(ptr2, 0);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_fopen)
{
	//def
	const char * fname = "/tmp/test-ummap-fopen-driver.txt";

	//map 2
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_fopen(fname, "w+"), NULL, "none");

	//we just write, no read pre-existing content
	ummap_skip_first_read(ptr1);

	//setup
	memset(ptr1, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umsync(ptr1, 0, 0);
	umunmap(ptr1, 0);

	//check
	FILE * fp = fopen(fname, "r");
	ASSERT_NE(nullptr, fp);
	char buffer[8*4096];
	ssize_t res = fread(buffer, 1, 8*4096, fp);
	ASSERT_EQ(8*4096, res);
	for (int i = 0 ; i < 8*4096 ; i++)
		ASSERT_EQ(10, buffer[i]) << "Index: " << i;
	
	//clear
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_dax_fopen)
{
	//def
	const char * fname = "/tmp/test-ummap-fopen-driver.txt";
	system("touch /tmp/test-ummap-fopen-driver.txt");
	truncate(fname, 8*4096);

	//map 2
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_dax_fopen(fname, "rw+", false), NULL, "none");

	//we just write, no read pre-existing content
	ummap_skip_first_read(ptr1);

	//setup
	memset(ptr1, 'a', 8*4096);

	//unmap 1 and let the other for cleaup
	umsync(ptr1, 0, 0);
	umunmap(ptr1, 0);

	//check
	FILE * fp = fopen(fname, "r");
	ASSERT_NE(nullptr, fp);
	char buffer[8*4096];
	ssize_t res = fread(buffer, 1, 8*4096, fp);
	ASSERT_EQ(8*4096, res);
	for (int i = 0 ; i < 8*4096 ; i++)
		ASSERT_EQ('a', buffer[i]) << "Index: " << i;
	
	//clear
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_fd)
{
	//def
	const char * fname = "/tmp/test-ummap-fopen-driver.txt";

	//open
	FILE * fp = fopen(fname, "w+");
	ASSERT_NE(nullptr, fp);
	int fd = fileno(fp);
	ftruncate(fd, 8*4096);

	//map 2
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_fd(fd), NULL, "none");
	fclose(fp);

	//we just write, no read pre-existing content
	ummap_skip_first_read(ptr1);

	//setup
	memset(ptr1, 'a', 8*4096);

	//unmap 1 and let the other for cleaup
	umsync(ptr1, 0, 0);
	umunmap(ptr1, 0);

	//check
	fp = fopen(fname, "r");
	ASSERT_NE(nullptr, fp);
	char buffer[8*4096];
	ssize_t res = fread(buffer, 1, 8*4096, fp);
	ASSERT_EQ(8*4096, res);
	for (int i = 0 ; i < 8*4096 ; i++)
		ASSERT_EQ('a', buffer[i]) << "Index: " << i;
	
	//clear
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_dax_fd)
{
	//def
	const char * fname = "/tmp/test-ummap-dax-fopen-driver.txt";

	//open
	FILE * fp = fopen(fname, "w+");
	ASSERT_NE(nullptr, fp);
	int fd = fileno(fp);
	ftruncate(fd, 8*4096);

	//map 2
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_dax_fd(fd, false), NULL, "none");
	fclose(fp);

	//we just write, no read pre-existing content
	ummap_skip_first_read(ptr1);

	//setup
	memset(ptr1, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umsync(ptr1, 0, 0);
	umunmap(ptr1, 0);

	//check
	fp = fopen(fname, "r");
	ASSERT_NE(nullptr, fp);
	char buffer[8*4096];
	ssize_t res = fread(buffer, 1, 8*4096, fp);
	ASSERT_EQ(8*4096, res);
	for (int i = 0 ; i < 8*4096 ; i++)
		ASSERT_EQ(10, buffer[i]) << "Index: " << i;
	
	//clear
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_dax_fd_offset)
{
	//def
	const char * fname = "/tmp/test-ummap-dax-fopen-driver-offset.txt";

	//open
	FILE * fp = fopen(fname, "w+");
	ASSERT_NE(nullptr, fp);
	int fd = fileno(fp);
	ftruncate(fd, 8*4096);

	//map 2
	const size_t offset = 256;
	void * ptr1 = ummap(NULL, 8*4096 - offset, 4096, offset, PROT_READ|PROT_WRITE, 0, ummap_driver_create_dax_fd(fd, true), NULL, "none");
	fclose(fp);

	//we just write, no read pre-existing content
	ummap_skip_first_read(ptr1);

	//setup
	memset(ptr1, 10, 8*4096 - offset);

	//unmap 1 and let the other for cleaup
	umsync(ptr1, 0, 0);
	umunmap(ptr1, 0);

	//check
	fp = fopen(fname, "r");
	ASSERT_NE(nullptr, fp);
	char buffer[8*4096];
	ssize_t res = fread(buffer, 1, 8*4096, fp);
	ASSERT_EQ(8*4096, res);
	for (int i = 0 ; i < offset ; i++)
		ASSERT_EQ(0, buffer[i]) << "Index: " << i;
	for (int i = offset ; i < 8*4096 ; i++)
		ASSERT_EQ(10, buffer[i]) << "Index: " << i;
	
	//clear
	unlink(fname);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_memory_ok)
{
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_memory(8*4096), NULL, "none");
	memset(ptr1, 10, 8*4096);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1, 0);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_memory_not_ok)
{
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_memory(4*4096), NULL, "none");
	ASSERT_DEATH(memset(ptr1, 10, 8*4096),"overpass");

	//unmap 1 and let the other for cleaup
	umunmap(ptr1, 0);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, map_driver_dummy)
{
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_dummy(64), NULL, "none");
	memset(ptr1, 10, 8*4096);

	//check
	for (int i = 0 ; i < 8*4096 ; i++)
		ASSERT_EQ(10, ((char*)ptr1)[i]);

	//unmap 1 and let the other for cleaup
	umunmap(ptr1, 0);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, destroy_driver)
{
	ummap_driver_t * driver = ummap_driver_create_dummy(64);
	ummap_driver_destroy(driver);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, autoclean_driver)
{
	ummap_driver_t * driver = ummap_driver_create_dummy(64);
	ummap_driver_set_autoclean(driver, false);
	ummap_driver_destroy(driver);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, policy_group)
{
	//setup policies
	ummap_policy_t * globalPolicy = ummap_policy_create_fifo(4*4096, false);
	ummap_policy_t * localPolicy = ummap_policy_create_fifo(4*4096, true);

	//reg global
	ummap_policy_group_register("global", globalPolicy);

	//map
	void * ptr1 = ummap(NULL, 8*4096, 4096, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_dummy(64), localPolicy, "global");
	memset(ptr1, 0 , 8*4096);

	//unmap
	umunmap(ptr1, 0);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, lbm_bugguy_case_1)
{
	//set buggy size non multiple of 1MB
	size_t size = 37140768;

	//map
	void * ptr1 = ummap(NULL, size, 1024*1024, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_dummy(64), NULL, "none");

	//touch
	#pragma omp parallel
	for (size_t i = 0 ; i < size ; i++)
		((char*)ptr1)[i] = 34;

	//unam
	umunmap(ptr1, 0);
}

/*******************  FUNCTION  *********************/
void test_create_uri(const char * uri)
{
	//set buggy size non multiple of 1MB
	size_t segmentSize = 4096;
	size_t size = 8*segmentSize;

	//map
	void * ptr1 = ummap(NULL, size, segmentSize, 0, PROT_READ|PROT_WRITE, 0, ummap_driver_create_uri(uri), NULL, "none");

	//touch
	for (size_t i = 0 ; i < size ; i++)
		((char*)ptr1)[i] = 34;

	//unam
	umunmap(ptr1, 0);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, create_driver_uri)
{
	test_create_uri("file:///tmp/test-uri.txt?mode=w+");
	test_create_uri("dummy://16");
	test_create_uri("mem://32768");
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, create_policy_uri)
{
	ummap_policy_t * policy;
	
	//fifo
	policy = ummap_policy_create_uri("fifo://4096", true);
	ummap_policy_destroy(policy);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, cow_fopen)
{
	//driver
	const size_t segmentSize = 4096;
	const size_t size = 8*segmentSize;

	//create & memset
	ummap_driver_t * driver = ummap_driver_create_fopen("/tmp/ummap-io-api-test-cow-fopen-1.raw", "w+");
	void * ptr = ummap(NULL, size, segmentSize, 0, PROT_READ|PROT_WRITE, 0, driver, NULL, "none");
	memset(ptr, 1, size);
	umsync(ptr, size, false);

	//cow
	int status = ummap_cow_fopen(ptr, "/tmp/ummap-io-api-test-cow-fopen-2.raw", "w+", true);
	ASSERT_EQ(0, status);

	//write again & flush
	memset(ptr, 2, size/2);
	umunmap(ptr, true);

	//map again orign & check
	ummap_driver_t * driver2 = ummap_driver_create_fopen("/tmp/ummap-io-api-test-cow-fopen-1.raw", "r");
	char* ptr2 = (char*)ummap(NULL, size, segmentSize, 0, PROT_READ, 0, driver2, NULL, "none");
	for (size_t i = 0 ; i < size ; i++)
		ASSERT_EQ(1, ptr2[i]);
	
	//map again orign & check
	ummap_driver_t * driver3 = ummap_driver_create_fopen("/tmp/ummap-io-api-test-cow-fopen-2.raw", "r");
	char * ptr3 = (char*)ummap(NULL, size, segmentSize, 0, PROT_READ, 0, driver3, NULL, "none");
	for (size_t i = 0 ; i < size / 2 ; i++)
		ASSERT_EQ(2, ptr3[i]);
	for (size_t i = size / 2 ; i < size ; i++)
		ASSERT_EQ(1, ptr3[i]);
	
	//unmap
	umunmap(ptr2, size);
	umunmap(ptr3, size);

	//clean
	unlink("/tmp/ummap-io-api-test-cow-fopen-1.raw");
	unlink("/tmp/ummap-io-api-test-cow-fopen-2.raw");
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, cow_dax_fopen)
{
	//driver
	const size_t segmentSize = 4096;
	const size_t size = 8*segmentSize;

	//create & memset
	ummap_driver_t * driver = ummap_driver_create_dax_fopen("/tmp/ummap-io-api-test-cow-mmap-fopen-1.raw", "w+", true);
	void * ptr = ummap(NULL, size, segmentSize, 0, PROT_READ|PROT_WRITE, 0, driver, NULL, "none");
	memset(ptr, 1, size);
	umsync(ptr, size, false);

	//cow
	int status = ummap_cow_dax_fopen(ptr, "/tmp/ummap-io-api-test-cow-mmap-fopen-2.raw", "w+", true);
	ASSERT_EQ(0, status);

	//write again & flush
	memset(ptr, 2, size/2);
	umunmap(ptr, true);

	//map again orign & check
	ummap_driver_t * driver2 = ummap_driver_create_dax_fopen("/tmp/ummap-io-api-test-cow-mmap-fopen-1.raw", "r", true);
	char* ptr2 = (char*)ummap(NULL, size, segmentSize, 0, PROT_READ, 0, driver2, NULL, "none");
	for (size_t i = 0 ; i < size ; i++)
		ASSERT_EQ(1, ptr2[i]);
	
	//map again orign & check
	ummap_driver_t * driver3 = ummap_driver_create_dax_fopen("/tmp/ummap-io-api-test-cow-mmap-fopen-2.raw", "r", true);
	char * ptr3 = (char*)ummap(NULL, size, segmentSize, 0, PROT_READ, 0, driver3, NULL, "none");
	for (size_t i = 0 ; i < size / 2 ; i++)
		ASSERT_EQ(2, ptr3[i]);
	for (size_t i = size / 2 ; i < size ; i++)
		ASSERT_EQ(1, ptr3[i]);

	//unmap
	umunmap(ptr2, size);
	umunmap(ptr3, size);

	//clean
	unlink("/tmp/ummap-io-api-test-cow-mmap-fopen-1.raw");
	unlink("/tmp/ummap-io-api-test-cow-mmap-fopen-2.raw");
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, cow_uri)
{
	//vars
	ummap_driver_t * driver;
	const size_t segmentSize = 4096;
	const size_t size = 8*segmentSize;
	
	//dummy
	driver = ummap_driver_create_uri("dummy://0");
	void * ptr = ummap(NULL, size, segmentSize, 0, PROT_READ|PROT_WRITE, 0, driver, NULL, "none");
	ummap_cow_uri(ptr, "dummy://0", true);
	umunmap(ptr, false);
}

/*******************  FUNCTION  *********************/
TEST_F(TestPublicAPI, switch_uri)
{
	//vars
	ummap_driver_t * driver;
	const size_t segmentSize = 4096;
	const size_t size = 8*segmentSize;
	
	//dummy
	driver = ummap_driver_create_uri("dummy://0");
	void * ptr = ummap(NULL, size, segmentSize, 0, PROT_READ|PROT_WRITE, 0, driver, NULL, "none");
	ummap_switch_uri(ptr, "dummy://0", UMMAP_NO_ACTION);
	umunmap(ptr, false);

	//memory
	driver = ummap_driver_create_uri("mem://32MB");
	ptr = ummap(NULL, size, segmentSize, 0, PROT_READ|PROT_WRITE, 0, driver, NULL, "none");
	ummap_switch_uri(ptr, "mem://32MB", UMMAP_NO_ACTION);
	umunmap(ptr, false);
}
