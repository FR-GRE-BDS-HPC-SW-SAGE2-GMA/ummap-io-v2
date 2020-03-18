/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//google test
#include <gtest/gtest.h>
//local
#include "../FifoPolicy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
TEST(TestFifoPolicy, constructor)
{
	FifoPolicy policy(10*1024*1024, true);
}
