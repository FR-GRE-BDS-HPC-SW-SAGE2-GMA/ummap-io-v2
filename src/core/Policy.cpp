/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <mutex>
//internal
#include "../common/Debug.hpp"
//local
#include "Policy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
Policy::Policy(size_t maxMemory, bool local)
{
	this->maxMemory = maxMemory;
	this->local = local;
}

/*******************  FUNCTION  *********************/
Policy::~Policy(void)
{

}

/*******************  FUNCTION  *********************/
void Policy::registerMapping(Mapping * mapping, void * storage, size_t elementCount)
{
	//setup
	PolicyStorage entry = {
		.mapping = mapping,
		.storage = storage,
		.elementCount = elementCount
	};

	//register, CRITICAL SECTION
	{
		std::lock_guard<Spinlock> lockGuard(this->storageRegistryLock);
		this->storageRegistry.push_back(entry);
	}
}

/*******************  FUNCTION  *********************/
bool Policy::contains(PolicyStorage & storage, void * entry)
{
	return (entry >= storage.storage && entry < (char*)storage.storage + storage.elementCount);
}

/*******************  FUNCTION  *********************/
PolicyStorage Policy::getStorageInfo(void * entry)
{
	//start CRITICAL SECTION
	std::lock_guard<Spinlock> lockGuard(this->storageRegistryLock);

	//loop to search
	for (auto it : this->storageRegistry) {
		if (contains(it, entry))
			return it;
	}

	//not found
	UMMAP_FATAL("Fail to found policy storage entry !");
	PolicyStorage res = {0,0,0};
	return res;
}

/*******************  FUNCTION  *********************/
PolicyStorage Policy::getStorageInfo(Mapping * mapping)
{
	if (local) {
		assume(this->storageRegistry.size() == 1, "Invalid local list with multiple mapping registered !");
		return this->storageRegistry.front();
	} else {
		//start CRITICAL SECTION
		std::lock_guard<Spinlock> lockGuard(this->storageRegistryLock);

		//loop to search
		for (auto it : this->storageRegistry) {
			if (it.mapping == mapping)
				return it;
		}
	}

	//not found
	UMMAP_FATAL("Fail to found policy of given mapping !");
	PolicyStorage res = {0,0,0};
	return res;
}

/*******************  FUNCTION  *********************/
void Policy::unregisterMapping(Mapping * mapping)
{
	//start CRITICAL SECTION
	std::lock_guard<Spinlock> lockGuard(this->storageRegistryLock);

	//loop
	for (auto it = storageRegistry.begin() ; it != storageRegistry.end() ; ++it) {
		if (it->mapping == mapping)
			storageRegistry.erase(it);
	}
}
