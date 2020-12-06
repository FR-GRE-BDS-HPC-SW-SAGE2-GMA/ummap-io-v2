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
#include "FifoWindowPolicy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
FifoWindowPolicy::FifoWindowPolicy(size_t maxMemory, size_t maxSlidingMemory, bool local = false)
	:Policy(maxMemory, local)
{
	this->currentFixedMemory = 0;
	this->currentSlidingWindowMemory = 0;
	this->maxSlidingMemory = maxSlidingMemory;
	this->maxFixedMemory = maxMemory - maxSlidingMemory;
}

/*******************  FUNCTION  *********************/
FifoWindowPolicy::~FifoWindowPolicy(void)
{

}

/*******************  FUNCTION  *********************/
void FifoWindowPolicy::allocateElementStorage(Mapping * mapping, size_t segmentCount)
{
	//allocate
	FifoWindowPolicyMeta * meta = new FifoWindowPolicyMeta;
	meta->elements = new ListElement[segmentCount];
	meta->isFixedMemory = new bool[segmentCount];

	//set default
	memset(meta->isFixedMemory, 0, sizeof(bool) * segmentCount);

	//register
	this->registerMapping(mapping, meta, segmentCount, sizeof(ListElement));
}

/*******************  FUNCTION  *********************/
void FifoWindowPolicy::freeElementStorage(Mapping * mapping)
{
	//CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

		//get
		//@TODO get & unregister
		PolicyStorage storage = this->getStorageInfo(mapping);

		//remove all from list
		FifoWindowPolicyMeta * meta = static_cast<FifoWindowPolicyMeta*>(storage.elements);
		for (size_t i = 0 ; i < storage.elementCount ; i++) {
			ListElement & cur = meta->elements[i];
			if (cur.isInList()) {
				if (meta->isFixedMemory[i])
					this->currentFixedMemory -= mapping->getSegmentSize();
				else
					this->currentSlidingWindowMemory -= mapping->getSegmentSize();
			}
			cur.removeFromList();
		}

		//unregister
		this->unregisterMapping(mapping);

		//free
		delete [] meta->isFixedMemory;
		delete [] meta->elements;
		delete meta;
	}
}

/*******************  FUNCTION  *********************/
void FifoWindowPolicy::notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty)
{
	//CRITICAL SECTION
	{
		//take lock
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

		//get storage
		PolicyStorage storage = this->getStorageInfo(mapping);

		//get element
		FifoWindowPolicyMeta * meta = static_cast<FifoWindowPolicyMeta*>(storage.elements);
		ListElement * elements = meta->elements;
		ListElement & cur = elements[index];

		//check if is new touch
		bool isFirstAccess = cur.isAlone();

		//remove from list
		cur.removeFromList();

		//impact counters
		if (!isFirstAccess) {
			if (meta->isFixedMemory[index]) {
				this->currentFixedMemory -= mapping->getSegmentSize();
				assert(this->currentFixedMemory >= 0);
			} else {
				this->currentSlidingWindowMemory -= mapping->getSegmentSize();
				assert(this->currentSlidingWindowMemory >= 0);
			}
		}

		//select mode
		bool isFixed = (this->currentFixedMemory < this->maxFixedMemory);
		meta->isFixedMemory[index] = isFixed;
	
		//insert in list
		if (isFixed) {
			rootFixedWindow.insertAfter(cur);
			this->currentFixedMemory += mapping->getSegmentSize();
		} else {
			rootSlidingWindow.insertAfter(cur);
			this->currentSlidingWindowMemory += mapping->getSegmentSize();
		}

		//if too large, evict one
		while (this->currentSlidingWindowMemory > this->maxSlidingMemory ) {
			ListElement * toEvict = this->rootSlidingWindow.popPrev();
			if (toEvict != NULL) {
				//get infos related to segment
				PolicyStorage evictInfos = getStorageInfo(toEvict);

				//calc id
				FifoWindowPolicyMeta * meta = static_cast<FifoWindowPolicyMeta*>(evictInfos.elements);
				size_t id = toEvict - meta->elements;

				//evict
				evictInfos.mapping->evict(this, id);

				//update status
				this->currentSlidingWindowMemory -= evictInfos.mapping->getSegmentSize();
			}
		}
	}
}

/*******************  FUNCTION  *********************/
void FifoWindowPolicy::notifyEvict(Mapping * mapping, size_t index)
{
	//evict
	//CRITICAL SECTION
	{
		//take lock
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

		//get storage infos
		PolicyStorage storage = getStorageInfo(mapping);

		//get element
		FifoWindowPolicyMeta * meta = static_cast<FifoWindowPolicyMeta*>(storage.elements);
		ListElement * elements = meta->elements;
		ListElement & cur = elements[index];

		//check
		assert(cur.isInList());

		//impact counters
		if (meta->isFixedMemory[index]) {
			this->currentFixedMemory -= mapping->getSegmentSize();
			assert(this->currentFixedMemory >= 0);
		} else {
			this->currentSlidingWindowMemory -= mapping->getSegmentSize();
			assert(this->currentSlidingWindowMemory >= 0);
		}

		//remove from list
		cur.removeFromList();
	}
}


/*******************  FUNCTION  *********************/
bool FifoWindowPolicy::contains(PolicyStorage & storage, void * entry)
{
	FifoWindowPolicyMeta * meta = static_cast<FifoWindowPolicyMeta*>(storage.elements);
	ListElement * elements = meta->elements;
	return (entry >= elements && entry < (char*)elements + (storage.elementCount * storage.elementSize));
}
