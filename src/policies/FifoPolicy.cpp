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
FifoPolicy::FifoPolicy(size_t maxMemory, bool local)
	:Policy(maxMemory, local)
{

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
		std::lock_guard<Spinlock> lockGuard(this->rootLock);

		//get
		//@TODO get & unregister
		PolicyStorage storage = this->getStorageInfo(mapping);

		//remove all from list
		ListElement * elts = static_cast<ListElement*>(storage.storage);
		for (size_t i = 0 ; i < storage.elementCount ; i++) {
			ListElement & cur = elts[i];
			cur.removeFromList();
		}

		//unregister
		this->unregisterMapping(mapping);

		//free
		delete [] elts;
	}
}

/*******************  FUNCTION  *********************/
void FifoPolicy::notifyTouch(Mapping * mapping, size_t index, bool isWrite)
{
	//get storage
	PolicyStorage storage = this->getStorageInfo(mapping);

	//get element
	ListElement * elts = static_cast<ListElement*>(storage.storage);
	ListElement & cur = elts[index];

	//check if is new touch
	bool isFirstAccess = cur.isAlone();

	//CRITICAL SECTION
	{
		//take lock
		std::lock_guard<Spinlock> lockGuard(this->rootLock);

		//remove from list
		cur.removeFromList();
	
		//insert in list head
		
	}


}
