/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_FIFO_WINDOW_POLICY_HPP
#define UMMAP_FIFO_WINDOW_POLICY_HPP

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
 * Implement a FIFO policy with a sliding window. We first fill the fixed window and when it
 * is full we let it as it is and use the sliding window to fill/evict.
**/
class FifoWindowPolicy : public Policy
{
	public:
		FifoWindowPolicy(size_t maxMemory, size_t maxSlidingMemory, bool local);
		virtual ~FifoWindowPolicy(void);
		virtual void allocateElementStorage(Mapping * mapping, size_t segmentCount) override;
		virtual void notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty) override;
		virtual void notifyEvict(Mapping * mapping, size_t index) override;
		virtual void freeElementStorage(Mapping * mapping) override;
		virtual size_t getCurrentMemory(void) override;
		virtual void shrinkMemory(void) override;
	protected:
		/** Root element of the fixed window **/
		ListElement rootFixedWindow;
		/** Root element of the sliding window when the fixed one if full. **/
		ListElement rootSlidingWindow;
		/** 
		 * Keep track of the memory used by the fixed window. When full we 
		 * register to the sliding window.
		**/
		size_t currentFixedMemory;
		/**
		 * Keep track of the memory used by the sliding window.
		**/
		size_t currentSlidingWindowMemory;
		/** Maximum memory allowed by the sliding window. **/
		size_t maxSlidingMemory;
		/** Maximum memory allowed by the fixed window. **/
		size_t maxFixedMemory;
};

}

#endif //UMMAP_FIFO_WINDOW_POLICY_HPP
