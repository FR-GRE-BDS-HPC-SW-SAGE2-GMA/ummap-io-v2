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
//local
#include "Policy.hpp"
#include "PolicyQuotaLocal.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Constructor of the local process policy quota. It assign the static maximal memory this quota
 * allow on the policies. This implementation share the memory between policies inside the current
 * process.
**/
PolicyQuotaLocal::PolicyQuotaLocal(size_t staticMaxMemory)
	:PolicyQuota(staticMaxMemory)
{
}

/*******************  FUNCTION  *********************/
size_t PolicyQuotaLocal::getUsedMemory(void) const
{
	//counter
	size_t totMem = 0;

	//loop all
	for (auto & it : this->policies)
		totMem += it->getCurrentMemory();

	//ret
	return totMem;
}

/*******************  FUNCTION  *********************/
/**
 * Implement the update policy to distribute the quotas over the policies.
 * Here I reused the original algorithm from Sergio/KTH on the V1 version
 * of ummap-io by dividing by 2 the memory of the largest memory user
 * until we fit in the global limit
**/
void PolicyQuotaLocal::update(void)
{
	//CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::mutex> lockGuard(this->mutex);

		//compute target
		size_t cntPolicies = this->policies.size();

		//nothing to do
		printf("cnt %d %zu\n", cntPolicies, this->staticMaxMemory);
		if (cntPolicies == 0)
			return;

		//calc average limit
		size_t averageMem = this->staticMaxMemory / cntPolicies;
		
		//get current memory
		size_t totMem = this->getUsedMemory();
		printf("ok %zu %zu\n", totMem, staticMaxMemory);

		//apply reduction until we reach acceptale solution
		while (totMem > this->staticMaxMemory) {
			//search max
			Policy * polMax = *this->policies.begin();
			for (auto & it : this->policies)
				if (it->getCurrentMemory() > polMax->getCurrentMemory())
					polMax = it;

			//calc new limit
			size_t limit = averageMem + (polMax->getCurrentMemory() - averageMem) / 2;
			polMax->setDynamicMaxMemory(limit);
			printf("set limit : %zu\n", limit);
			polMax->shrinkMemory();

			//calc new tot mem
			totMem = this->getUsedMemory();
		}
	}
}
