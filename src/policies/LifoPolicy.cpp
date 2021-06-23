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
/**
 * Constructor of the LIFO policy.
 * @param maxMemory Maximum memory allowed before evicting pages.
 * @param local Define if it is a local policy so we can avoid some locks.
**/
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
		while (this->currentMemory > this->dynamicMaxMemory) {
			ListElement * toEvict = this->root.popPrev();
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
				this->currentMemory -= evictInfos.mapping->getSegmentSize();
			}
		}

		//insert in list
		root.insertBefore(cur);
	}

	//really do the evict out of the critical section to keep multi-threading
	//to write data
	for (size_t i = 0 ; i < cntIdsToEvict; i++)
		mappings[i]->evict(this, idsToEvict[i]);

	//notif quota to redistribute if needed
	if (isFirstAccess && this->policyQuota != NULL && this->currentMemory < quotaAverageLimitForNotif)
		this->policyQuota->update();
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

/*******************  FUNCTION  *********************/
void LifoPolicy::shrinkMemory(void)
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
		while (this->currentMemory > this->dynamicMaxMemory) {
			ListElement * toEvict = this->root.popPrev();
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
				this->currentMemory -= evictInfos.mapping->getSegmentSize();
			}
		}
	}

	//really do the evict out of the critical section to keep multi-threading
	//to write data
	for (size_t i = 0 ; i < cntIdsToEvict; i++)
		mappings[i]->evict(this, idsToEvict[i]);
}

/*******************  FUNCTION  *********************/
size_t LifoPolicy::getCurrentMemory(void)
{
	return this->currentMemory;
}
