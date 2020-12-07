/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <mutex>
#include <cassert>
//internal
#include "../common/Debug.hpp"
//local
#include "Policy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Policy constructor.
 * @param maxMemory Define the maximum memory to be allowed by this policy.
 * @param local Define if we are using a local of global policy to know how to use mutexes.
**/
Policy::Policy(size_t maxMemory, bool local)
{
	this->maxMemory = maxMemory;
	this->local = local;
	this->mutexPtr = &this->localMutex;
}

/*******************  FUNCTION  *********************/
/**
 * Destructor of the policy.
**/
Policy::~Policy(void)
{

}

/*******************  FUNCTION  *********************/
/**
 * Replace the current lock to be used by the given one. This is used
 * when using a local and global policy to share locking for eviction operations.
 * @param mutex The mutex to be used instead of the local one.
**/
void Policy::forceUsingGroupMutex(std::recursive_mutex * mutex)
{
	this->mutexPtr = mutex;
}

/*******************  FUNCTION  *********************/
/**
 * Get the local mutex.
**/
std::recursive_mutex * Policy::getLocalMutex(void)
{
	return &this->localMutex;
}

/*******************  FUNCTION  *********************/
/**
 * Register a new mapping to the policy.
 * It mostly allocate the new elements for status tracking.
**/
void Policy::registerMapping(Mapping * mapping, void * storage, size_t elementCount, size_t elementSize, void * extraInfos)
{
	//setup
	PolicyStorage entry = {
		.mapping = mapping,
		.elements = storage,
		.elementCount = elementCount,
		.elementSize = elementSize,
		.extraInfos = extraInfos,
	};

	//register, CRITICAL SECTION
	{
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);
		this->storageRegistry.push_back(entry);
	}
}

/*******************  FUNCTION  *********************/
/**
 * Check if the storage contain the given entry by looking in elements range.
**/
bool Policy::contains(PolicyStorage & storage, void * entry)
{
	return (entry >= storage.elements && entry < (char*)storage.elements + (storage.elementCount * storage.elementSize));
}

/*******************  FUNCTION  *********************/
/**
 * Get the storage info from the given entry.
 * @param entry The entry from which we want to storage info.
 * @return Return the storage info related to the given entry.
 * This function fail by exiting if not found.
**/
PolicyStorage Policy::getStorageInfo(void * entry)
{
	//vars
	PolicyStorage res = {0,0,0,0};

	//lock
	std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

	//loop to search
	for (auto it : this->storageRegistry) {
		if (contains(it, entry))
			res = it;
	}

	//not found
	if (res.mapping == NULL)
		UMMAP_FATAL("Fail to found policy storage entry !");
	
	//ret
	return res;
}

/*******************  FUNCTION  *********************/
/**
 * Get the storage info for the given mapping.
 * @param mapping Define the mapping for which we want the storage info;.
 * @return Return the storage info. If not found the function make the process
 * failing on error.
**/
PolicyStorage Policy::getStorageInfo(Mapping * mapping)
{
	//var
	PolicyStorage res = {0,0,0};

	if (local) {
		assume(this->storageRegistry.size() == 1, "Invalid local list with multiple mapping registered !");
		PolicyStorage res = this->storageRegistry.front();
		assert(res.mapping == mapping);
		return res;
	} else {
		//start CRITICAL SECTION
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

		//loop to search
		for (auto it : this->storageRegistry) {
			if (it.mapping == mapping)
				res = it;
		}
	}

	//not found
	if (res.mapping == NULL)
		UMMAP_FATAL("Fail to found policy of given mapping !");
	
	//ret
	return res;
}

/*******************  FUNCTION  *********************/
/**
 * unregister the given mapping.
 * @param mapping The mapping to unregister.
**/
void Policy::unregisterMapping(Mapping * mapping)
{
	//start CRITICAL SECTION
	std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

	//loop
	for (auto it = storageRegistry.begin() ; it != storageRegistry.end() ; ++it) {
		if (it->mapping == mapping) {
			auto tmp = it;
			++it;
			storageRegistry.erase(tmp);
		};
	}
}

/*******************  FUNCTION  *********************/
/**
 * Associate the URI used to build the policy to keep track and report it to htopml.
 * @param uri The URI as a string.
**/
void Policy::setUri(const std::string & uri)
{
	this->uri = uri;
}

/*******************  FUNCTION  *********************/
/**
 * Returnt he URI associated to the policy.
**/
const std::string & Policy::getUri(void) const
{
	return this->uri;
}
