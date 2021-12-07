/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_SPINLOCK_PTHREAD_HPP
#define UMMAP_SPINLOCK_PTHREAD_HPP

#ifndef __APPLE__

/********************  HEADERS  *********************/
#include "pthread.h"

/*******************  NAMESPACE  ********************/
namespace ummapio
{

/*********************  CLASS  **********************/
class SpinlockPthread
{
	public:
		SpinlockPthread();
		~SpinlockPthread();
		void lock();
		void unlock();
		bool tryLock();
	private:
		pthread_spinlock_t spinlock;
};

/*******************  FUNCTION  *********************/
inline SpinlockPthread::SpinlockPthread(void )
{
	pthread_spin_init(&spinlock,PTHREAD_PROCESS_PRIVATE);
}

/*******************  FUNCTION  *********************/
inline SpinlockPthread::~SpinlockPthread(void )
{
	pthread_spin_destroy(&spinlock);
}

/*******************  FUNCTION  *********************/
inline void SpinlockPthread::lock(void )
{
	pthread_spin_lock(&spinlock);
}

/*******************  FUNCTION  *********************/
inline void SpinlockPthread::unlock(void )
{
	pthread_spin_unlock(&spinlock);
}

/*******************  FUNCTION  *********************/
inline bool SpinlockPthread::tryLock(void )
{
	return pthread_spin_trylock(&spinlock);
}

}

#else

/********************  HEADERS  *********************/
#include <libkern/OSAtomic.h>

/*******************  NAMESPACE  ********************/
namespace ummap {

/*********************  CLASS  **********************/
class SpinlockPthread {
  OSSpinLock m_lock;

 public:
  SpinlockPthread()
      : m_lock(0) {
  }
  void lock() {
    OSSpinLockLock(&m_lock);
  }
  void unlock() {
    OSSpinLockUnlock(&m_lock);
  }
  bool tryLock() {
    return OSSpinLockTry(&m_lock);
  }
};

}

#endif

#endif //UMMAP_SPINLOCK_PTHREAD_HPP
