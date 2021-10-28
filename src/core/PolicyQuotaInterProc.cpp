/*****************************************************
			 PROJECT  : ummap-io-v2
			 VERSION  : 0.0.0-dev
			 DATE     : 03/2020
			 LICENSE  : ????????
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cassert>
//unix
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
//internal
#include "../common/Debug.hpp"
//local
#include "Policy.hpp"
#include "PolicyQuotaInterProc.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/********************  MACROS  **********************/
#define SHM_FFLAGS  (O_CREAT | O_RDWR)
#define SHM_FPERM   (S_IRUSR | S_IWUSR)

/********************  GLOBAL ***********************/
static PolicyQuotaInterProc * gblPolicyQuotaInterProc = NULL;

/*******************  FUNCTION  *********************/
static void sigHandler(int signum){
	if (gblPolicyQuotaInterProc != NULL)
		gblPolicyQuotaInterProc->update();
}


/*******************  FUNCTION  *********************/
PolicyQuotaInterProc::PolicyQuotaInterProc(const std::string & name,size_t staticMaxMemory)
                     :PolicyQuotaLocal(staticMaxMemory)
{
	//keep track
	this->name = std::string("/") + name;
	this->totalAllowed = staticMaxMemory;

	//set
	assume(gblPolicyQuotaInterProc == NULL, "Cannot instantiate mulitple inter-proc policy quota for now !");
	gblPolicyQuotaInterProc = this;

	//setup signal handler
	signal(SIGEVICT,sigHandler);

	//open
	this->shared = static_cast<PolicyQuotaInterProcShared*>(this->openShm(this->name, sizeof(*this->shared)));

	//CRITICAL SECTION
	{
		//take lock
		this->lock();

		//increment process number
		this->shared->processes++;

		//search a position in the list
		bool found = false;
		for (int i = 0 ; i < this->shared->indexMax ; ++i) {
			if (this->shared->pids[i] == 0 || kill(this->shared->pids[i],0) == 0) {
				this->shared->pids[i] = getpid();
				found = true;
				break;
			}
		}

		//if not found add
		if (found == false)
			this->shared->pids[++(this->shared->indexMax)] = getpid();

		//end of critical section
		this->unlock();
	}

	//signal all
	this->signalAll();

	//update
	this->update();
}

/*******************  FUNCTION  *********************/
PolicyQuotaInterProc::~PolicyQuotaInterProc(void)
{
	//CRITICAL SECTION
	{
		//take lock
		this->lock();

		//increment process number
		this->shared->processes--;

		//search a position in the list
		bool found = false;
		for (int i = 0 ; i < this->shared->indexMax ; ++i) {
			if (this->shared->pids[i] == getpid()) {
				this->shared->pids[i] = 0;
				found = true;
				break;
			}
		}

		//if not found add
		assume(found, "Fail to found the local PID in the shared->pid[] vector !");

		//end of critical section
		this->unlock();
	}

	//remove global registration
	gblPolicyQuotaInterProc = NULL;
}

/*******************  FUNCTION  *********************/
void PolicyQuotaInterProc::signalAll(void)
{
	//CRITICAL SECTION
	{
		//take lock
		this->lock();

		//loop on all to send signal
		for (int i = 0 ; i < this->shared->indexMax ; i++) {
			//check if still alive
			if (this->shared->pids[i] != 0 && kill(this->shared->pids[i],0) == 0) {
				this->shared->processes--;
				this->shared->pids[i] = 0;
			} else if (this->shared->pids[i] == getpid()) {
				//nothing to do, not want to send to self
			} else {
				//send signal
				sigval_t sigval = { .sival_ptr = NULL };
				sigqueue(this->shared->pids[i], SIGEVICT, sigval);
			}
		}

		//end of critical section
		this->unlock();
	}
}


/*******************  FUNCTION  *********************/
void PolicyQuotaInterProc::lock(void)
{
	int expected = 0;
	while(!this->shared->spinlock.compare_exchange_strong(expected,1)) {
		//spin until can take the lock
		expected = 0;
	}
}


/*******************  FUNCTION  *********************/
void PolicyQuotaInterProc::unlock()
{
	int expected = 1;
	bool status = this->shared->spinlock.compare_exchange_strong(expected,0);
	assume(status, "Fail to unlock, got invalid value on the spinlock !");
}

/*******************  FUNCTION  *********************/
void PolicyQuotaInterProc::update(void)
{
	//calc quota
	size_t perProcQuota = this->totalAllowed / this->shared->processes;
	this->staticMaxMemory = perProcQuota;

	//debug
	UMMAP_DEBUG_ARG("quota:interproc", "Change quota for %1 computed from %2 processes").argUnit1024(perProcQuota).arg(this->shared->processes).end();

	//call local update
	PolicyQuotaLocal::update();
}

/*******************  FUNCTION  *********************/
void * PolicyQuotaInterProc::openShm(const std::string &name, size_t size)
{
	//vars
	int32_t fd = -1;
	struct stat  st = { 0 };

	//check
	assert(size % 4096 == 0);

	//open
	fd = shm_open(name.c_str(), SHM_FFLAGS, SHM_FPERM);
	assumeArg(fd >= 0, "Failed to shm_open('%1'): %2").arg(name).argStrErrno().end();

	//stats
	int status = fstat(fd, &st);
	assumeArg(status == 0, "Failed to fstat the shm file '%1': %2").arg(name).argStrErrno().end();

	//truncate
	if (size > 0 && st.st_size == 0)
	{
		status = ftruncate(fd, size);
		assumeArg(status == 0, "Failed to ftruncate the shm file '%1': %2").arg(name).argStrErrno().end();
	}

	//map
	void * ptr = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
	assumeArg(ptr != MAP_FAILED, "Failed to mmap('%1'): %2").arg(name).argStrErrno().end();

	//close
	status = close(fd);
	assumeArg(status == 0, "Failed to close the shm file '%1': %2").arg(name).argStrErrno().end();

	//ret
	return ptr;
}
