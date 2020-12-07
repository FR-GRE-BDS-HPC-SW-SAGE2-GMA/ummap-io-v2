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
#include "LifoPolicy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
LifoPolicy::LifoPolicy(size_t maxMemory, bool local = false)
	:Policy(maxMemory, local)
{
	this->currentMemory = 0;
}

/*******************  FUNCTION  *********************/
LifoPolicy::~LifoPolicy(void)
{

}

/*******************  FUNCTION  *********************/
void LifoPolicy::allocateElementStorage(Mapping * mapping, size_t segmentCount)
{
	ListElement * elts = new ListElement[segmentCount];
	this->registerMapping(mapping, elts, segmentCount, sizeof(ListElement));
}

/*******************  FUNCTION  *********************/
void LifoPolicy::freeElementStorage(Mapping * mapping)
{
	//CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

		//get
		//@TODO get & unregister
		PolicyStorage storage = this->getStorageInfo(mapping);

		//remove all from list
		ListElement * elements = static_cast<ListElement*>(storage.elements);
		for (size_t i = 0 ; i < storage.elementCount ; i++) {
			ListElement & cur = elements[i];
			if (cur.isInList())
				this->currentMemory -= mapping->getSegmentSize();
			cur.removeFromList();
		}

		//unregister
		this->unregisterMapping(mapping);

		//free
		delete [] elements;
	}
}

/*******************  FUNCTION  *********************/
void LifoPolicy::notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty)
{
	//CRITICAL SECTION
	{
		//take lock
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

		//get storage
		PolicyStorage storage = this->getStorageInfo(mapping);

		//get element
		ListElement * elements = static_cast<ListElement*>(storage.elements);
		ListElement & cur = elements[index];

		//check if is new touch
		bool isFirstAccess = cur.isAlone();

		//remove from list
		cur.removeFromList();

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

		//insert in list
		root.insertBefore(cur);
	}
}

/*******************  FUNCTION  *********************/
void LifoPolicy::notifyEvict(Mapping * mapping, size_t index)
{
	//evict
	//CRITICAL SECTION
	{
		//take lock
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

		//get storage infos
		PolicyStorage storage = getStorageInfo(mapping);

		//get element
		ListElement * elements = static_cast<ListElement*>(storage.elements);
		ListElement & cur = elements[index];

		//check
		assert(cur.isInList());

		//decrease memory
		this->currentMemory -= mapping->getSegmentSize();
		assert(this->currentMemory >= 0);

		//remove from list
		cur.removeFromList();
	}
}
