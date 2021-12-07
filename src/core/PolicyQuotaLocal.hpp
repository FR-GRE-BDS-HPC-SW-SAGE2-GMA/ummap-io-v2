/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_POLICY_QUOTA_PROCESS_HPP
#define UMMAP_POLICY_QUOTA_PROCESS_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <list>
#include <string>
#include <mutex>
//internal
#include "PolicyQuota.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  STRUCT  *********************/
/**
 * Implement the quota sharing between policies inside the current process.
**/
class PolicyQuotaLocal : public PolicyQuota
{
	public:
		PolicyQuotaLocal(size_t staticMaxMemory);
		virtual void update(void) override;
	private:
		size_t getUsedMemory(void) const;
};

}

#endif //UMMAP_POLICY_QUOTA_PROCESS_HPP
