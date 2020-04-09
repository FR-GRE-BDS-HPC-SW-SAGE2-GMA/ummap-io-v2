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
void Policy::registerMapping(Mapping * mapping, void * storage, size_t elementCount, size_t elementSize)
{
	//setup
	PolicyStorage entry = {
		.mapping = mapping,
		.elements = storage,
		.elementCount = elementCount,
		.elementSize = elementSize
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
	return (entry >= storage.elements && entry < (char*)storage.elements + (storage.elementCount * storage.elementSize));
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
		PolicyStorage res = this->storageRegistry.front();
		assert(res.mapping == mapping);
		return res;
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
		if (it->mapping == mapping) {
			auto tmp = it;
			++it;
			storageRegistry.erase(tmp);
		};
	}
}

/*******************  FUNCTION  *********************/
void Policy::setUri(const std::string & uri)
{
	this->uri = uri;
}

/*******************  FUNCTION  *********************/
const std::string & Policy::getUri(void) const
{
	return this->uri;
}
