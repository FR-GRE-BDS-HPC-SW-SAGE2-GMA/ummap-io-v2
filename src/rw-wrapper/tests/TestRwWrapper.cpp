/*****************************************************
			 PROJECT  : ummap-io-v2
			 VERSION  : 0.0.0-dev
			 DATE     : 04/2020
			 LICENSE  : ????????
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include <sys/mman.h>
#include "../../public-api/ummap.h"
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE (1024*1024)
#define SEGMENT_SIZE (4096)

/*********************  CLASS  **********************/
class TestRwWrapper : public testing::Test
{
	public:
		virtual void SetUp(void);
		virtual void TearDown(void);
};

/*******************  FUNCTION  *********************/
void TestRwWrapper::SetUp(void)
{
	ummap_init();
}

/*******************  FUNCTION  *********************/
void TestRwWrapper::TearDown(void)
{
	ummap_finalize();
}

/*******************  FUNCTION  *********************/
TEST_F(TestRwWrapper, native_read)
{
	int fd = open("/dev/zero", O_RDONLY);
	char buffer[SIZE];
	ssize_t res = read(fd, buffer, SIZE);
	EXPECT_EQ(SIZE, res);
	close(fd);
}

/*******************  FUNCTION  *********************/
TEST_F(TestRwWrapper, native_write)
{
	int fd = open("/dev/null", O_WRONLY);
	char buffer[SIZE];
	ssize_t res = write(fd, buffer, SIZE);
	EXPECT_EQ(SIZE, res);
	close(fd);
}

/*******************  FUNCTION  *********************/
TEST_F(TestRwWrapper, read_on_ummap)
{
	//map
	ummap_driver_t * driver = ummap_driver_create_dummy(0);
	ummap_policy_t * policy = ummap_policy_create_fifo(2 * SEGMENT_SIZE, true);
	void * buffer = ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, driver, policy, NULL);
	memset(buffer, 0, SIZE);

	//open
	int fd = open("/dev/zero", O_RDONLY);

	//write & check
	ssize_t res = read(fd, buffer, SIZE);
	EXPECT_EQ(2 * SEGMENT_SIZE, res);

	//close & unmap
	close(fd);
	umunmap(buffer, false);
}

/*******************  FUNCTION  *********************/
TEST_F(TestRwWrapper, write_on_ummap)
{
	//map
	ummap_driver_t * driver = ummap_driver_create_dummy(0);
	ummap_policy_t * policy = ummap_policy_create_fifo(2 * SEGMENT_SIZE, true);
	void * buffer = ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, driver, policy, NULL);
	memset(buffer, 0, SIZE);

	//open
	int fd = open("/dev/null", O_WRONLY);

	//write & check
	ssize_t res = write(fd, buffer, SIZE);
	EXPECT_EQ(SIZE, res);

	//close & unmap
	close(fd);
	umunmap(buffer, false);
}

/*******************  FUNCTION  *********************/
TEST_F(TestRwWrapper, native_fread)
{
	FILE * fp = fopen("/dev/zero", "r");
	char buffer[SIZE];
	ssize_t res = fread(buffer, 1, SIZE, fp);
	EXPECT_EQ(SIZE, res);
	fclose(fp);
}

/*******************  FUNCTION  *********************/
TEST_F(TestRwWrapper, native_fwrite)
{
	FILE * fp = fopen("/dev/null", "w");
	char buffer[SIZE];
	ssize_t res = fwrite(buffer, 1, SIZE, fp);
	EXPECT_EQ(SIZE, res);
	fclose(fp);
}

/*******************  FUNCTION  *********************/
TEST_F(TestRwWrapper, fread_on_ummap)
{
	//map
	ummap_driver_t * driver = ummap_driver_create_dummy(0);
	ummap_policy_t * policy = ummap_policy_create_fifo(2 * SEGMENT_SIZE, true);
	void * buffer = ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, driver, policy, NULL);
	memset(buffer, 0, SIZE);

	//open
	FILE * fp = fopen("/dev/zero", "r");

	//write & check
	ssize_t res = fread(buffer, 2, SIZE / 2, fp);
	EXPECT_EQ(SIZE / 2, res);

	//close & unmap
	fclose(fp);
	umunmap(buffer, false);
}

/*******************  FUNCTION  *********************/
TEST_F(TestRwWrapper, fwrite_on_ummap)
{
	//map
	ummap_driver_t * driver = ummap_driver_create_dummy(0);
	ummap_policy_t * policy = ummap_policy_create_fifo(2 * SEGMENT_SIZE, true);
	void * buffer = ummap(NULL, SIZE, SEGMENT_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, driver, policy, NULL);
	memset(buffer, 0, SIZE);

	//open
	FILE * fp = fopen("/dev/null", "r+");

	//write & check
	ssize_t res = fwrite(buffer, 2, SIZE/2, fp);
	EXPECT_EQ(SIZE / 2, res);

	//close & unmap
	fclose(fp);
	umunmap(buffer, false);
}
