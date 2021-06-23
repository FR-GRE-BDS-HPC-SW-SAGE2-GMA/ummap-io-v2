/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <mutex>
#include <cassert>
#include <cstring>
//internal
#include "../common/Debug.hpp"
#include "../core/Mapping.hpp"
//local
#include "FifoWindowPolicy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Constructor of the fifo window policy.
 * @param maxMemory Maximum memory allowed by the policy (fixed + sliding window)
 * @param maxSlidingWindow Maximum meomry allowed by the sliding window (it must be 
 * deduced from the max memory to determine the fixed window size).
 * @param local Define if it is a local or global memory.
**/
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
	ListElement * elements = new ListElement[segmentCount];
	bool * extraInfos = new bool[segmentCount];

	//set default
	memset(extraInfos, 0, sizeof(bool) * segmentCount);

	//register
	this->registerMapping(mapping, elements, segmentCount, sizeof(ListElement), extraInfos);
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

		//extract
		ListElement * elements = static_cast<ListElement*>(storage.elements);
		bool * isFixedMemory = static_cast<bool*>(storage.extraInfos);

		//remove all from list
		for (size_t i = 0 ; i < storage.elementCount ; i++) {
			ListElement & cur = elements[i];
			if (cur.isInList()) {
				if (isFixedMemory[i])
					this->currentFixedMemory -= mapping->getSegmentSize();
				else
					this->currentSlidingWindowMemory -= mapping->getSegmentSize();
			}
			cur.removeFromList();
		}

		//unregister
		this->unregisterMapping(mapping);

		//free
		delete [] isFixedMemory;
		delete [] elements;
	}
}

/*******************  FUNCTION  *********************/
void FifoWindowPolicy::notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty)
{
	//vars
	const int maxIdsToEvict = 128;
	size_t idsToEvict[maxIdsToEvict];
	Mapping * mappings[maxIdsToEvict];
	int cntIdsToEvict = 0;
	bool isFirstAccess = false;

	//CRITICAL SECTION
	{
		//take lock
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

		//get storage
		PolicyStorage storage = this->getStorageInfo(mapping);

		//get element
		bool * isFixedMemory = static_cast<bool*>(storage.extraInfos);
		ListElement * elements = static_cast<ListElement*>(storage.elements);
		ListElement & cur = elements[index];

		//check if is new touch
		isFirstAccess = cur.isAlone();

		//remove from list
		cur.removeFromList();

		//impact counters
		if (!isFirstAccess) {
			if (isFixedMemory[index]) {
				this->currentFixedMemory -= mapping->getSegmentSize();
				assert(this->currentFixedMemory >= 0);
			} else {
				this->currentSlidingWindowMemory -= mapping->getSegmentSize();
				assert(this->currentSlidingWindowMemory >= 0);
			}
		}

		//select mode
		bool isFixed = (this->currentFixedMemory < this->maxFixedMemory);
		isFixedMemory[index] = isFixed;
	
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
				idsToEvict[cntIdsToEvict] = toEvict - (ListElement*)evictInfos.elements;

				//keep track of the mapping
				mappings[cntIdsToEvict] = evictInfos.mapping;

				//inc counter
				cntIdsToEvict++;

				//this can append only if sharing policy over segments with different
				//segment size.
				assume(cntIdsToEvict < maxIdsToEvict, "Reach maximum pages to evict due to optimization, cannot continue !");

				//update status
				this->currentSlidingWindowMemory -= evictInfos.mapping->getSegmentSize();
			}
		}
	}

	//really do the evict out of the critical section to keep multi-threading
	//to write data
	for (size_t i = 0 ; i < cntIdsToEvict; i++)
		mappings[i]->evict(this, idsToEvict[i]);

	//notif quota to redistribute if needed
	if (isFirstAccess && this->policyQuota != NULL && this->getCurrentMemory() < quotaAverageLimitForNotif)
		this->policyQuota->update();
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
		bool * isFixedMemory = static_cast<bool*>(storage.extraInfos);
		ListElement * elements = static_cast<ListElement*>(storage.elements);
		ListElement & cur = elements[index];

		//check
		assert(cur.isInList());

		//impact counters
		if (isFixedMemory[index]) {
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
void FifoWindowPolicy::shrinkMemory(void)
{
	//vars
	const int maxIdsToEvict = 128;
	size_t idsToEvict[maxIdsToEvict];
	Mapping * mappings[maxIdsToEvict];
	int cntIdsToEvict = 0;

	//CRITICAL SECTION
	{
		//take lock
		std::lock_guard<std::recursive_mutex> lockGuard(*this->mutexPtr);

		//if too large, evict one
		while (this->currentSlidingWindowMemory > this->maxSlidingMemory ) {
			ListElement * toEvict = this->rootSlidingWindow.popPrev();
			if (toEvict != NULL) {
				//get infos related to segment
				PolicyStorage evictInfos = getStorageInfo(toEvict);

				//calc id
				idsToEvict[cntIdsToEvict] = toEvict - (ListElement*)evictInfos.elements;

				//keep track of the mapping
				mappings[cntIdsToEvict] = evictInfos.mapping;

				//inc counter
				cntIdsToEvict++;

				//this can append only if sharing policy over segments with different
				//segment size.
				assume(cntIdsToEvict < maxIdsToEvict, "Reach maximum pages to evict due to optimization, cannot continue !");

				//update status
				this->currentSlidingWindowMemory -= evictInfos.mapping->getSegmentSize();
			}
		}
	}

	//really do the evict out of the critical section to keep multi-threading
	//to write data
	for (size_t i = 0 ; i < cntIdsToEvict; i++)
		mappings[i]->evict(this, idsToEvict[i]);
}

/*******************  FUNCTION  *********************/
size_t FifoWindowPolicy::getCurrentMemory(void)
{
	return this->currentSlidingWindowMemory  + this->currentFixedMemory;
}
