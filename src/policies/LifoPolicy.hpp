/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_LIFO_POLICY_HPP
#define UMMAP_LIFO_POLICY_HPP

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
 * Provide the implementation of a LIFO (Last In, First Out) policy.
**/
class LifoPolicy : public Policy
{
	public:
		LifoPolicy(size_t maxMemory, bool local);
		virtual ~LifoPolicy(void);
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

#endif //UMMAP_LIFO_POLICY_HPP
