/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_SPINLOCK_APPLE_HPP
#define UMMAP_SPINLOCK_APPLE_HPP

/********************  HEADERS  *********************/
#include <libkern/OSAtomic.h>

/*******************  NAMESPACE  ********************/
namespace ummapio {

/*********************  CLASS  **********************/
class SpinlockApple {
  OSSpinLock m_lock;

 public:
  SpinlockApple()
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

#endif //UMMAP_SPINLOCK_APPLE_HPP
