/*****************************************************
			 PROJECT  : ummap-io-v2
			 VERSION  : 0.0.0-dev
			 DATE     : 02/2020
			 LICENSE  : ????????
*****************************************************/

#ifndef UMMAP_POLICY_QUOTA_INTER_PROCESS_HPP
#define UMMAP_POLICY_QUOTA_INTER_PROCESS_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <list>
#include <string>
#include <mutex>
#include <atomic>
//internal
#include "PolicyQuota.hpp"
#include "PolicyQuotaLocal.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  STRUCT  *********************/
#define SHARED_TAB_ENTRIES ((1024 * 1024 - 4 * sizeof(std::atomic<int>)) / sizeof(pid_t))
#define SIGEVICT SIGUSR1
struct PolicyQuotaInterProcShared
{
	std::atomic<int> padding;
	std::atomic<int> spinlock;
	std::atomic<int> processes;
	std::atomic<int> indexMax;
	pid_t pids[SHARED_TAB_ENTRIES];
};

/*********************  STRUCT  *********************/
/**
 * Implement the quota sharing between policies between processes.
 * The sharing is made with a simple rule, each process get 1/Nth of
 * the total allowed memory.
**/
class PolicyQuotaInterProc : public PolicyQuotaLocal
{
	public:
		PolicyQuotaInterProc(const std::string & name,size_t staticMaxMemory);
		~PolicyQuotaInterProc(void);
		virtual void update(void) override;
		static void deferSignal(void);
		static void runDeferedSignal(void);
	private:
		void * openShm(const std::string &name, size_t size);
		void signalAll(void);
		void lock(void);
		void unlock(void);
	private:
		std::string name;
		PolicyQuotaInterProcShared * shared;
		size_t totalAllowed;
};

}

#endif //UMMAP_POLICY_QUOTA_INTER_PROCESS_HPP
