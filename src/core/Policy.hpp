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
#include <string>
#include <mutex>

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
class Mapping;

/*********************  STRUCT  *********************/
/**
 * Define a policy storage joining a mapping with is tracking elements.
**/
struct PolicyStorage
{
	/**
	 * Mapping attached to the policy list elements.
	**/
	Mapping * mapping;
	/**
	 * Elements to track the segment status. This is commonly a list of next/prev elements.
	**/
	void * elements;
	/**
	 * Number of elements in the mapping.
	**/
	size_t elementCount;
	/**
	 * Size of a an element.
	**/
	size_t elementSize;
	/**
	 * Optional extra infos to be used by the policy implementation.
	**/
	void * extraInfos;
};

/*********************  CLASS  **********************/
class Policy
{
	public:
		Policy(size_t maxMemory, bool local);
		virtual ~Policy(void);
		virtual void allocateElementStorage(Mapping * mapping, size_t segmentCount) = 0;
		virtual void notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty) = 0;
		virtual void notifyEvict(Mapping * mapping, size_t index) = 0;
		virtual void freeElementStorage(Mapping * mapping) = 0;
		void setUri(const std::string & uri);
		const std::string & getUri(void) const;
		void forceUsingGroupMutex(std::recursive_mutex * mutex);
		std::recursive_mutex * getLocalMutex(void);
	protected:
		void registerMapping(Mapping * mapping, void * storage, size_t elementCount, size_t elementSize, void * extraInfos = NULL);
		void unregisterMapping(Mapping * mapping);
		PolicyStorage getStorageInfo(void * entry);
		PolicyStorage getStorageInfo(Mapping * mapping);
		static bool contains(PolicyStorage & storage, void * entry);
	protected:
		bool local;
		size_t maxMemory;
		std::list<PolicyStorage> storageRegistry;
		std::recursive_mutex * mutexPtr;
		std::recursive_mutex localMutex;
		std::string uri;
};

}

#endif //UMMAP_POLICY_HPP
