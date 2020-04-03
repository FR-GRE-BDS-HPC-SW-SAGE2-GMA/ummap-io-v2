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
class FifoPolicy : public Policy
{
	public:
		FifoPolicy(size_t maxMemory, bool local);
		virtual ~FifoPolicy(void);
		virtual void allocateElementStorage(Mapping * mapping, size_t segmentCount) override;
		virtual void notifyTouch(Mapping * mapping, size_t index, bool isWrite, bool mapped, bool dirty) override;
		virtual void notifyEvict(Mapping * mapping, size_t index) override;
		virtual void freeElementStorage(Mapping * mapping) override;
	protected:
		ListElement root;
		std::mutex rootLock;
		size_t currentMemory;
};

}

#endif //UMMAP_FIFO_POLICY_HPP
