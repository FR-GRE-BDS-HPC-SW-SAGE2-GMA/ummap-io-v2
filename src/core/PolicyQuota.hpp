/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_POLICY_QUOTA_HPP
#define UMMAP_POLICY_QUOTA_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <list>
#include <string>
#include <mutex>

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
class Policy;

/*********************  STRUCT  *********************/
/**
 * Define a quota to be able to balance memory usage over several policies in
 * a balanced way opposite to the uncontrolled approach offerd by the policy
 * groups.
**/
class PolicyQuota
{
	public:
		PolicyQuota(size_t staticMaxMemory);
		virtual ~PolicyQuota(void);
		virtual void update(void) = 0;
		void registerPolicy(Policy * policy);
		void unregisterPolicy(Policy * policy);
		size_t getStaticMaxMemory(void) const {return this->staticMaxMemory;};
	private:
		void updateNotifyLimit(void);
	protected:
		/** Keep track of maximum amount of memory to attach to this quota component. **/
		size_t staticMaxMemory;
		/** Protect the access to the policy list. **/
		std::mutex mutex;
		/** Keep track of the policies between each to balance. **/
		std::list<Policy *> policies;
};

}

#endif //UMMAP_POLICY_QUOTA_HPP
