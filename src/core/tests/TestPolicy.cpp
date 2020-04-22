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

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*********************  CLASS  **********************/
class PublicPolicy: public Policy
{
	public:
		PublicPolicy(void) : Policy(4096, false) {};
		virtual void allocateElementStorage(Mapping * mapping, size_t segmentCount) {};
		virtual void notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty) {};
		virtual void notifyEvict(Mapping * mapping, size_t index) {};
		virtual void freeElementStorage(Mapping * mapping) {};
		void registerMapping(Mapping * mapping, void * storage, size_t elementCount, size_t elementSize) {Policy::registerMapping(mapping, storage, elementCount, elementSize);};
		void unregisterMapping(Mapping * mapping) {Policy::unregisterMapping(mapping);};
		PolicyStorage getStorageInfo(void * entry) {return Policy::getStorageInfo(entry);};
		PolicyStorage getStorageInfo(Mapping * mapping) {return Policy::getStorageInfo(mapping);};
		static bool contains(PolicyStorage & storage, void * entry) {return Policy::contains(storage, entry);};
};

/*******************  FUNCTION  *********************/
TEST(TestPolicy, Constructor)
{
	PublicPolicy policy;
}

/*******************  FUNCTION  *********************/
TEST(TestPolicy, contains)
{
	const int size = 4096;
	char * buffer = (char*)malloc(size);

	PolicyStorage storage = {
		.mapping = (Mapping *)0x1,
		.elements = buffer,
		.elementCount= size,
		.elementSize = 1,
	};

	//ok
	ASSERT_TRUE(PublicPolicy::contains(storage, buffer));
	ASSERT_TRUE(PublicPolicy::contains(storage, buffer+ size - 1));

	//not ok
	ASSERT_FALSE(PublicPolicy::contains(storage, buffer - 1));
	ASSERT_FALSE(PublicPolicy::contains(storage, buffer+ size));

	//free
	free(buffer);
}

/*******************  FUNCTION  *********************/
TEST(TestPolicy, register)
{
	//build
	PublicPolicy policy;
	Mapping * mapping = (Mapping *)0x1;
	const int size = 4096;
	char buffer[size];

	//register
	policy.registerMapping(mapping, buffer, size, 1);

	//check
	PolicyStorage out1 = policy.getStorageInfo(mapping);
	EXPECT_EQ(out1.mapping, mapping);
	EXPECT_EQ(out1.elements, buffer);
}

/*******************  FUNCTION  *********************/
TEST(TestPolicy, getStorageInfo_mapping)
{
	//build
	PublicPolicy policy;
	Mapping * mapping = (Mapping *)0x1;
	const int size = 4096;
	char buffer[size];

	//register
	policy.registerMapping(mapping, buffer, size, 1);

	//check
	PolicyStorage out1 = policy.getStorageInfo(mapping);
	EXPECT_EQ(out1.mapping, mapping);
	EXPECT_EQ(out1.elements, buffer);

	ASSERT_DEATH(policy.getStorageInfo(mapping + 1), "Fail to found policy of given mapping !");
}

/*******************  FUNCTION  *********************/
TEST(TestPolicy, getStorageInfo_element)
{
	//build
	PublicPolicy policy;
	Mapping * mapping = (Mapping *)0x1;
	const int size = 4096;
	char buffer[size];

	//register
	policy.registerMapping(mapping, buffer, size, 1);

	//check
	PolicyStorage out1 = policy.getStorageInfo(buffer);
	EXPECT_EQ(out1.mapping, mapping);
	EXPECT_EQ(out1.elements, buffer);

	//check
	PolicyStorage out2 = policy.getStorageInfo(buffer + size - 1);
	EXPECT_EQ(out2.mapping, mapping);
	EXPECT_EQ(out2.elements, buffer);

	ASSERT_DEATH(policy.getStorageInfo(buffer + size), "Fail to found policy storage entry !");
}
