/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_FIFO_POLICY_HPP
#define UMMAP_FIFO_POLICY_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <mutex>
#include <list>
//local
#include "common/ListElement.hpp"
#include "core/Policy.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
class Mapping;

/*********************  CLASS  **********************/
/**
 * Provide the implementation of a FIFO (First In, First Out) policy.
**/
class FifoPolicy : public Policy
{
	public:
		FifoPolicy(size_t maxMemory, bool local);
		virtual ~FifoPolicy(void);
		virtual void allocateElementStorage(Mapping * mapping, size_t segmentCount) override;
		virtual void notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty) override;
		virtual void notifyEvict(Mapping * mapping, size_t index) override;
		virtual void freeElementStorage(Mapping * mapping) override;
		virtual size_t getCurrentMemory(void) override;
		virtual void shrinkMemory(void) override;
	protected:
		/** Root element of the linked list of segments to track.**/
		ListElement root;
		/** Keep track of the current memory usage.**/
		size_t currentMemory;
};

}

#endif //UMMAP_FIFO_POLICY_HPP
