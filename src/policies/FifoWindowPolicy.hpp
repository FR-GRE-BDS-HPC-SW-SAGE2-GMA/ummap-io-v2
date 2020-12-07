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
class FifoWindowPolicy : public Policy
{
	public:
		FifoWindowPolicy(size_t maxMemory, size_t maxSlidingMemory, bool local);
		virtual ~FifoWindowPolicy(void);
		virtual void allocateElementStorage(Mapping * mapping, size_t segmentCount) override;
		virtual void notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty) override;
		virtual void notifyEvict(Mapping * mapping, size_t index) override;
		virtual void freeElementStorage(Mapping * mapping) override;
	protected:
		ListElement rootFixedWindow;
		ListElement rootSlidingWindow;
		size_t currentFixedMemory;
		size_t currentSlidingWindowMemory;
		size_t maxSlidingMemory;
		size_t maxFixedMemory;
};

}

#endif //UMMAP_FIFO_WINDOW_POLICY_HPP