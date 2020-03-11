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
//local
#include "portability/Spinlock.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
class Mapping;

/*********************  STRUCT  *********************/
struct PolicyStorage
{
	Mapping * mapping;
	void * storage;
	size_t size;
};

/*********************  CLASS  **********************/
class Policy
{
	public:
		Policy(size_t maxMemory, bool local);
		virtual ~Policy(void);
		virtual void * allocateElementStorage(Mapping * mapping, size_t segmentCount) = 0;
		virtual void notifyTouch(void * storage, size_t index, bool isWrite) = 0;
		virtual void notifyEvict(void * storage, size_t index) = 0;
		virtual void freeElementStorage(void * storage, size_t segmentCount) = 0;
	protected:
		void registerMapping(Mapping * mapping, void * storage, size_t size);
		void unregisterMapping(void * storage, size_t size);
		PolicyStorage getStorageInfo(void * entry);
		static bool contains(PolicyStorage & storage, void * entry);
	protected:
		bool local;
		size_t maxMemory;
		std::list<PolicyStorage> storageRegistry;
		Spinlock storageRegistryLock;
};

}

#endif //UMMAP_POLICY_HPP
