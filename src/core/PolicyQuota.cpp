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
#include "PolicyQuota.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * Constructor of the policy quota. It assign the static maximal memory this quota
 * allow on the policies.
**/
PolicyQuota::PolicyQuota(size_t staticMaxMemory)
{
	this->staticMaxMemory = staticMaxMemory;
}

/*******************  FUNCTION  *********************/
/**
 * Destructor of the policy quota.
**/
PolicyQuota::~PolicyQuota(void)
{
	//start CRITICAL SECTION
	std::lock_guard<std::mutex> lockGuard(this->mutex);

	//check
	assume(this->policies.empty(), "Try to delete a policy quota which still have policies registered to it !");
}

/*******************  FUNCTION  *********************/
/**
 * Register a new policy to the policy quota.
 * @param policy Pointer to the policy to register.
**/
void PolicyQuota::registerPolicy(Policy * policy)
{
	//check
	assume(policy != NULL, "Invalid NULL policy recived for quota registration !");

	//register in CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::mutex> lockGuard(this->mutex);

		//register in list
		this->policies.push_back(policy);

		//set back link
		policy->setQuota(this);
	}

	//update
	this->updateNotifyLimit();
	this->update();
}

/*******************  FUNCTION  *********************/
/**
 * Unregister a policy from the policy list attached to this quota tracker.
 * @param policy Address of the policy to remove.
**/
void PolicyQuota::unregisterPolicy(Policy * policy)
{
	//check
	assume(policy != NULL, "Invalid NULL policy recived for quota de-registration !");

	//register in CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::mutex> lockGuard(this->mutex);

		//search
		bool found = false;
		for (auto it = policies.begin() ; it != policies.end() ; ++it) {
			if (*it == policy) {
				policies.erase(it);
				found = true;
				break;
			}
		}

		//check
		assume(found, "Did not found the policy to remove in the registed list !");

		//remove
		policy->setQuota(NULL);
	}

	//update
	this->updateNotifyLimit();
	this->update();
}

/*******************  FUNCTION  *********************/
/**
 * We want to be notify it the policy use less memory than the notify limit
 * and it increase. This is because in this case we need to possibly redistribute
 * the memory used by each other policies if some use more memory than the allowed
 * average.
**/
void PolicyQuota::updateNotifyLimit(void)
{
	//compute target
	size_t cntPolicies = this->policies.size();

	//nothing to do
	if (cntPolicies == 0)
		return;

	//calc average limit
	size_t averageMem = this->staticMaxMemory / cntPolicies;

	//dispatch
	for (auto & it : this->policies) {
		it->setDynamicMaxMemory(this->staticMaxMemory);
		it->setNotifyQuotaOnIncrease(averageMem);
	}
}