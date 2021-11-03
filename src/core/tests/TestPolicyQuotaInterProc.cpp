/*****************************************************
			 PROJECT  : ummap-io-v2
			 VERSION  : 0.0.0-dev
			 DATE     : 03/2020
			 LICENSE  : ????????
*****************************************************/

/********************  HEADERS  *********************/
//gtest
#include <gtest/gtest.h>
//local
#include "../Mapping.hpp"
#include "../Policy.hpp"
#include "../../policies/FifoPolicy.hpp"
#include "../PolicyQuotaInterProc.hpp"
#include "../../portability/OS.hpp"

/********************  MACROS  **********************/
#define PAGE_SIZE 4096

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestPolicyQuotaInterProc, basic)
{
	PolicyQuotaInterProc quota("policy-test", 4*UMMAP_PAGE_SIZE);
	FifoPolicy * policy = new FifoPolicy(4*UMMAP_PAGE_SIZE, true);
	size_t size = 8*UMMAP_PAGE_SIZE;

	//register
	quota.registerPolicy(policy);

	//update
	quota.update();

	//check
	EXPECT_EQ(4*UMMAP_PAGE_SIZE, quota.getStaticMaxMemory());

	//delete policy
	delete policy;
}

/*******************  FUNCTION  *********************/
TEST(TestPolicyQuotaInterProc, two_process)
{
	const int NBPROC = 2;
	int pid = fork();

	PolicyQuotaInterProc * quota = new PolicyQuotaInterProc("policy-test", 4*UMMAP_PAGE_SIZE);
	FifoPolicy * policy = new FifoPolicy(4*UMMAP_PAGE_SIZE, true);
	size_t size = 8*UMMAP_PAGE_SIZE;

	//register
	quota->registerPolicy(policy);

	//sleep
	sleep(1);

	//update
	quota->update();

	//sleep
	sleep(1);

	//check
	EXPECT_EQ((4*UMMAP_PAGE_SIZE)/NBPROC, quota->getStaticMaxMemory());

	//sleep
	sleep(1);

	//delete policy
	delete policy;
	delete quota;

	int status = 0;
	if (pid != 0)
		waitpid(pid, &status, 0);
	else
		exit(0);

	//check
	EXPECT_EQ(0, status);
}
