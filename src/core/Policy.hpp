/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_POLICY_HPP
#define UMMAP_POLICY_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <list>

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
class Mapping;

/*********************  STRUCT  *********************/
struct PolicyStorage
{
	Mapping * mapping;
	void * base;
	size_t size;
};

/*********************  CLASS  **********************/
class Policy
{
	public:
		Policy(size_t maxMemory);
		virtual ~Policy(void);
		virtual void * getElementStorage(Mapping * mapping, size_t segmentCount) = 0;
		virtual void touch(void * storage, size_t index, bool isWrite) = 0;
		virtual void evict(void * storage, size_t index) = 0;
		virtual void freeElementStorage(void * storage, size_t segmentCount);
	protected:
		void registerMapping(Mapping * mapping, void * storage, size_t size);
		PolicyStorage getStorageInfo(void * entry);
	protected:
		size_t maxMemory;
		std::list<PolicyStorage> storageRegistry;
};

}

#endif //UMMAP_POLICY_HPP
