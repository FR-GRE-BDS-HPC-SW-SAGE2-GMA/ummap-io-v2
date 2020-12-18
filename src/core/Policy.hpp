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
/**
 * Base class to imlement a policy. A policy is an implementation providing
 * touch and evict methods to be able to keep control on the memory consumption
 * of a given mapping.
 * A policy can be local of global so it might use locks to secure accesses.
**/
class Policy
{
	public:
		Policy(size_t maxMemory, bool local);
		virtual ~Policy(void);
		/**
		 * Allocate the element storage to keep track of the pages for the given
		 * new memory mapping.
		 * @param mapping The new memory mapping.
		 * @param segmentCount The number of segments in the mapping.
		**/
		virtual void allocateElementStorage(Mapping * mapping, size_t segmentCount) = 0;
		/**
		 * Notify a memory access to a segment to register it in the eviction list.
		 * @param mapping The mapping of the segment.
		 * @param index Index of the segment in the mapping.
		 * @param isWrite True if we are handling a write memory access
		 * @param mapped If already mapped or a new mapping.
		 * @param dirty If the segment was dirty.
		**/
		virtual void notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty) = 0;
		/**
		 * Notify an eviction from the mapping to update lists. This can append when
		 * the other (local or global) policy generate an eviction.
		 * @param mapping Define the mapping for which we evict the segment.
		 * @param index Define the index of the segment to evict.
		**/
		virtual void notifyEvict(Mapping * mapping, size_t index) = 0;
		/**
		 * Cleanup the element storage linked to the given memory mapping.
		 * @param mapping The mapping we want to untrack.
		**/
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
		/** Define if the policy is a local of global policy shared between multiple mappings. **/
		bool local;
		/** Keep track of the maximum memory allowed by the current policy. **/
		size_t maxMemory;
		/** Keep track of state storage attached to each handle mappings. **/
		std::list<PolicyStorage> storageRegistry;
		/** 
		 * Shared mutex to protect the access to the local states. It is a pointer so
		 * we can share the mutex between the local and global policy if both are used.
		**/
		std::recursive_mutex * mutexPtr;
		/**
		 * Local policy to be pointed by mutexPtr if local only and shared with all interacting policies
		 * if we are in a shared policy.
		**/
		std::recursive_mutex localMutex;
		/** Keeo track of the URI for htopml. **/
		std::string uri;
};

}

#endif //UMMAP_POLICY_HPP
