/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_FIFO_POLICY_HPP
#define UMMAP_FIFO_POLICY_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <list>
//local
#include "portability/Spinlock.hpp"
#include "core/Policy.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
class Mapping;

/*********************  STRUCT  *********************/
struct ListElement
{
	//funcs
	ListElement(void);
	//members
	ListElement * prev;
	ListElement * next;
};

/*********************  CLASS  **********************/
class FifoPolicy : public Policy
{
	public:
		FifoPolicy(size_t maxMemory, bool local);
		virtual ~FifoPolicy(void);
		virtual void allocateElementStorage(Mapping * mapping, size_t segmentCount);
		virtual void notifyTouch(void * storage, size_t index, bool isWrite);
		virtual void notifyEvict(void * storage, size_t index) = 0;
		virtual void freeElementStorage(Mapping * mapping);
	protected:
		ListElement root;
		Spinlock rootLock;
};

}

#endif //UMMAP_FIFO_POLICY_HPP
