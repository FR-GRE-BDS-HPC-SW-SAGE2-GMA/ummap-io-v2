/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//gtest
#include <gtest/gtest.h>
//unix
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//local
#include "portability/OS.hpp"
#include "../FDDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap_io;

/*******************  FUNCTION  *********************/
TEST(TestFDDriver, constructor)
{
	std::string fname = "/tmp/ummap-io-v2-test-fd-driver-constr.txt";
	int fd = open(fname.c_str(), O_APPEND|O_CREAT, S_IRWXU);
	OS::removeFile(fname);
	ASSERT_GT(fd, 0);
	FDDriver driver(fd);
	close(fd);
}

/*******************  FUNCTION  *********************/
TEST(TestFDDriver, pwrite_pread)
{
	std::string fname = "/tmp/ummap-io-v2-test-fd-driver-dup.txt";
	int fd = open(fname.c_str(), O_RDWR|O_APPEND|O_CREAT, S_IRWXU);
	OS::removeFile(fname);
	ASSERT_GT(fd, 0);
	FDDriver driver(fd);
	close(fd);

	char buffer1[4096];
	memset(buffer1, 10, sizeof(buffer1));
	ssize_t res1 = driver.pwrite(buffer1, sizeof(buffer1),0);
	fprintf(stderr, "%s\n", strerror(errno));
	ASSERT_EQ(res1, sizeof(buffer1));

	char buffer2[4096];
	ssize_t res2 = driver.pread(buffer2, sizeof(buffer2),0);
	ASSERT_EQ(res2, sizeof(buffer2));

	for (int i = 0 ; i < sizeof(buffer2) ; i++)
		ASSERT_EQ(10, buffer2[i]) << "Index : " << i;
}
