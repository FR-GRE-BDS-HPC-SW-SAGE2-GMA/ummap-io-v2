/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//gtest
#include <gtest/gtest.h>
//local
#include "../Mapping.hpp"
#include "../Policy.hpp"
#include "../../portability/OS.hpp"
#include "../../drivers/DummyDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*********************  CLASS  **********************/
class PublicPolicy: public Policy
{
	public:
		PublicPolicy(void) : Policy(4096, false) {};
		virtual void allocateElementStorage(Mapping * mapping, size_t segmentCount) override {};
		virtual void notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty) override {};
		virtual void notifyEvict(Mapping * mapping, size_t index) override {};
		virtual void freeElementStorage(Mapping * mapping) override {};
		void registerMapping(Mapping * mapping, void * storage, size_t elementCount, size_t elementSize) {Policy::registerMapping(mapping, storage, elementCount, elementSize);};
		void unregisterMapping(Mapping * mapping) {Policy::unregisterMapping(mapping);};
		PolicyStorage getStorageInfo(void * entry) {return Policy::getStorageInfo(entry);};
		PolicyStorage getStorageInfo(Mapping * mapping) {return Policy::getStorageInfo(mapping);};
		virtual bool contains(PolicyStorage & storage, void * entry) {return Policy::contains(storage, entry);};
		virtual size_t getCurrentMemory(void) override {return 0;}
		virtual void shrinkMemory(void) override {};
};

/*******************  FUNCTION  *********************/
TEST(TestPolicy, Constructor)
{
	PublicPolicy policy;
}

/*******************  FUNCTION  *********************/
TEST(TestPolicy, contains)
{
	//vars
	const int size = 4096;
	char * buffer = (char*)malloc(size);

	//mapping
	DummyDriver driver(0);
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//fill
	PolicyStorage storage = {
		&mapping,
		buffer,
		size,
		1,
		NULL,
	};

	//policy
	PublicPolicy policy;

	//ok
	ASSERT_TRUE(policy.contains(storage, buffer));
	ASSERT_TRUE(policy.contains(storage, buffer+ size - 1));

	//not ok
	ASSERT_FALSE(policy.contains(storage, buffer - 1));
	ASSERT_FALSE(policy.contains(storage, buffer+ size));

	//free
	free(buffer);
}

/*******************  FUNCTION  *********************/
TEST(TestPolicy, register)
{
	//build
	PublicPolicy policy;
	const int size = 4096;
	char buffer[size];

	//mapping
	DummyDriver driver(0);
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//register
	policy.registerMapping(&mapping, buffer, size, 1);

	//check
	PolicyStorage out1 = policy.getStorageInfo(&mapping);
	EXPECT_EQ(out1.mapping, &mapping);
	EXPECT_EQ(out1.elements, buffer);
}

/*******************  FUNCTION  *********************/
TEST(TestPolicy, getStorageInfo_mapping)
{
	//build
	PublicPolicy policy;
	const int size = 4096;
	char buffer[size];

	//mapping
	DummyDriver driver(0);
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//register
	policy.registerMapping(&mapping, buffer, size, 1);

	//check
	PolicyStorage out1 = policy.getStorageInfo(&mapping);
	EXPECT_EQ(out1.mapping, &mapping);
	EXPECT_EQ(out1.elements, buffer);

	ASSERT_DEATH(policy.getStorageInfo((&mapping) + 1), "Fail to found policy of given mapping !");
}

/*******************  FUNCTION  *********************/
TEST(TestPolicy, getStorageInfo_element)
{
	//build
	PublicPolicy policy;
	const int size = 4096;
	char buffer[size];

	//mapping
	DummyDriver driver(0);
	Mapping mapping(NULL, size, UMMAP_PAGE_SIZE, 0, PROT_READ|PROT_WRITE, UMMAP_DEFAULT, &driver, NULL, NULL);

	//register
	policy.registerMapping(&mapping, buffer, size, 1);

	//check
	PolicyStorage out1 = policy.getStorageInfo(buffer);
	EXPECT_EQ(out1.mapping, &mapping);
	EXPECT_EQ(out1.elements, buffer);

	//check
	PolicyStorage out2 = policy.getStorageInfo(buffer + size - 1);
	EXPECT_EQ(out2.mapping, &mapping);
	EXPECT_EQ(out2.elements, buffer);

	ASSERT_DEATH(policy.getStorageInfo(buffer + size), "Fail to found policy storage entry !");
}
