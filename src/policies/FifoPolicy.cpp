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
#include "../core/Mapping.hpp"
//local
#include "FifoPolicy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
FifoPolicy::FifoPolicy(size_t maxMemory, bool local)
	:Policy(maxMemory, local)
{
	this->currentMemory = 0;
}

/*******************  FUNCTION  *********************/
FifoPolicy::~FifoPolicy(void)
{

}

/*******************  FUNCTION  *********************/
void FifoPolicy::allocateElementStorage(Mapping * mapping, size_t segmentCount)
{
	ListElement * elts = new ListElement[segmentCount];
	this->registerMapping(mapping, elts, segmentCount);
}

/*******************  FUNCTION  *********************/
void FifoPolicy::freeElementStorage(Mapping * mapping)
{
	//CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::mutex> lockGuard(this->rootLock);

		//get
		//@TODO get & unregister
		PolicyStorage storage = this->getStorageInfo(mapping);

		//remove all from list
		ListElement * elements = static_cast<ListElement*>(storage.elements);
		for (size_t i = 0 ; i < storage.elementCount ; i++) {
			ListElement & cur = elements[i];
			cur.removeFromList();
		}

		//unregister
		this->unregisterMapping(mapping);

		//free
		delete [] elements;
	}
}

/*******************  FUNCTION  *********************/
void FifoPolicy::notifyTouch(Mapping * mapping, size_t index, bool isWrite)
{
	//get storage
	PolicyStorage storage = this->getStorageInfo(mapping);

	//get element
	ListElement * elements = static_cast<ListElement*>(storage.elements);
	ListElement & cur = elements[index];

	//check if is new touch
	bool isFirstAccess = cur.isAlone();

	//CRITICAL SECTION
	{
		//take lock
		std::lock_guard<std::mutex> lockGuard(this->rootLock);

		//remove from list
		cur.removeFromList();
	
		//insert in list
		root.insertAfter(cur);

		//increment memorr
		if (isFirstAccess)
			this->currentMemory += mapping->getSegmentSize();

		//if too large, evict one
		while (this->currentMemory > this->maxMemory) {
			ListElement * toEvict = this->root.popPrev();
			if (toEvict != NULL) {
				//get infos related to segment
				PolicyStorage evictInfos = getStorageInfo(toEvict);

				//calc id
				size_t id = toEvict - (ListElement*)evictInfos.elements;

				//evict
				evictInfos.mapping->evict(this, id);

				//update status
				this->currentMemory -= evictInfos.mapping->getSegmentSize();
			}
		}
	}
}

/*******************  FUNCTION  *********************/
void FifoPolicy::notifyEvict(Mapping * mapping, size_t index)
{
	//get storage infos
	PolicyStorage storage = getStorageInfo(mapping);

	//get element
	ListElement * elements = static_cast<ListElement*>(storage.elements);
	ListElement & cur = elements[index];

	//evict
	//CRITICAL SECTION
	{
		//take lock
		std::lock_guard<std::mutex> lockGuard(this->rootLock);

		//check
		assert(cur.isInList());

		//decrease memory
		this->currentMemory -= mapping->getSegmentSize();
		assert(this->currentMemory >= 0);

		//remove from list
		cur.removeFromList();
	}
}
