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
#include "FifoPolicy.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
ListElement::ListElement(void)
{
	this->next = this->prev = this;
}

/*******************  FUNCTION  *********************/
FifoPolicy::FifoPolicy(size_t maxMemory, bool local)
	:Policy(maxMemory, local)
{

}

/*******************  FUNCTION  *********************/
FifoPolicy::~FifoPolicy(void)
{

}

/*******************  FUNCTION  *********************/
void * FifoPolicy::allocateElementStorage(Mapping * mapping, size_t segmentCount)
{
	ListElement * elts = new ListElement[segmentCount];
	this->registerMapping(mapping, elts, segmentCount);
	return elts;
}

/*******************  FUNCTION  *********************/
void FifoPolicy::freeElementStorage(void * storage, size_t segmentCount)
{
	//CRITICAL SECTION
	{
		//lock
		std::lock_guard<Spinlock> lockGuard(this->rootLock);

		//remove all from list
		ListElement * elts = static_cast<ListElement*>(storage);
		for (size_t i = 0 ; i < segmentCount ; i++) {
			ListElement & cur = elts[i];
			//remove
			cur.next->prev = cur.prev;
			cur.prev->next = cur.next;
			//reset
			cur.next = cur.prev = &cur;
		}

		//unregister
		this->unregisterMapping(storage, segmentCount);

		//free
		delete [] elts;
	}
}
