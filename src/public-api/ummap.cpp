/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cerrno>
//unix
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//local
#include <config.h>
#include "../common/Debug.hpp"
#include "../common/HumanUnits.hpp"
#include "../core/GlobalHandler.hpp"
#include "../uri/MeroRessource.hpp"
#include "../uri/IocRessource.hpp"
#include "../drivers/FDDriver.hpp"
#include "../drivers/MemoryDriver.hpp"
#include "../drivers/DummyDriver.hpp"
#include "../drivers/MmapDriver.hpp"
#include "../drivers/CDriver.hpp"
#if defined(HAVE_MERO) || defined(HAVE_MOTR)
	#include "../drivers/ClovisDriver.hpp"
	#include "clovis_api.h"
#endif
#ifdef HAVE_IOC_CLIENT
	#include "../drivers/IocDriver.hpp"
#endif
#include "../policies/FifoPolicy.hpp"
#include "../policies/FifoWindowPolicy.hpp"
#include "../policies/LifoPolicy.hpp"
#include "../core/PolicyQuotaLocal.hpp"
#include "../core/PolicyQuotaInterProc.hpp"
#include "ummap.h"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  GLOBALS  **********************/
static int gblCntInit = 0;

/*******************  FUNCTION  *********************/
void ummap_init(void)
{
	//if not already init
	if (gblCntInit == 0) {
		ummapio::GlobalHandler * handler = new ummapio::GlobalHandler();
		ummapio::setGlobalHandler(handler);
		ummapio::setupSegfaultHandler();
	}

	//incr
	gblCntInit++;
}

/*******************  FUNCTION  *********************/
void ummap_finalize(void)
{
	//decr
	gblCntInit--;

	//destroy
	if (gblCntInit == 0) {
		ummapio::unsetSegfaultHandler();
		ummapio::clearGlobalHandler();
	}
}

/*******************  FUNCTION  *********************/
void * ummap(void * addr, size_t size, size_t segment_size, size_t storage_offset, int protection, int flags, ummap_driver_t * driver, ummap_policy_t * local_policy, const char * policy_group)
{
	//check
	assert(getGlobalhandler() != NULL);

	//convert
	Driver * driv = (Driver*)(driver);
	Policy * pol = (Policy*)(local_policy);

	//group
	const char * policy_group_checked = policy_group;
	if (policy_group_checked == NULL)
		policy_group_checked = "none";

	//call & ret
	return getGlobalhandler()->ummap(addr, size, segment_size, storage_offset, protection, flags, driv, pol, policy_group_checked);
}

/*******************  FUNCTION  *********************/
int umunmap(void * ptr, bool sync)
{
	//check
	assert(getGlobalhandler() != NULL);
	assert(ptr != NULL);

	//call
	return getGlobalhandler()->uunmap(ptr, sync);
}

/*******************  FUNCTION  *********************/
void ummap_skip_first_read(void * ptr)
{
	//check
	assert(getGlobalhandler() != NULL);
	assert(ptr != NULL);

	//call
	return getGlobalhandler()->skipFirstRead(ptr);
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_dummy(char value)
{
	Driver * driver = new DummyDriver(value);
	driver->setAutoclean(true);
	return (ummap_driver_t*)driver;
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_fopen(const char * file_path, const char * mode)
{
	//open
	FILE * fp = fopen(file_path, mode);
	assumeArg(fp != NULL, "Fail to open file '%1': %2").arg(file_path).argStrErrno().end();
	
	//create driver
	ummap_driver_t * driver = ummap_driver_create_fd(fileno(fp));
	ummap_driver_set_autoclean(driver, true);

	//close
	fclose(fp);

	//return
	return driver;
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_dax_fopen(const char * file_path, const char * mode, bool allowNotAligned)
{
	//open
	FILE * fp = fopen(file_path, mode);
	assumeArg(fp != NULL, "Fail to open file '%1': %2").arg(file_path).argStrErrno().end();
	
	//create driver
	ummap_driver_t * driver = ummap_driver_create_dax_fd(fileno(fp), allowNotAligned);
	ummap_driver_set_autoclean(driver, true);

	//close
	fclose(fp);

	//return
	return driver;
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_fd(int fd)
{
	Driver * driver = new FDDriver(fd);
	driver->setAutoclean(true);
	return (ummap_driver_t*)driver;
}


/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_clovis(int64_t high, int64_t low, bool create)
{
	#if defined(HAVE_MERO) || defined(HAVE_MOTR)
		struct m0_uint128 object_id = {.u_hi = high, .u_lo = low};
		ClovisDriver * driver = new ClovisDriver(object_id, create);
		driver->setAutoclean(true);
		return (ummap_driver_t*)driver;
	#else
		UMMAP_FATAL("Try to create a clovis driver, but ummap was compiled without mero/motr !");
		return NULL;
	#endif
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_dax_fd(int fd, bool allowNotAligned)
{
	Driver * driver = new MmapDriver(fd, allowNotAligned);
	driver->setAutoclean(true);
	return (ummap_driver_t*)driver;
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_memory(size_t size)
{
	Driver * driver = new MemoryDriver(size);
	driver->setAutoclean(true);
	return (ummap_driver_t*)driver;
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_ioc(ioc_client_t * client, int64_t high, int64_t low, bool create)
{
	#ifdef HAVE_IOC_CLIENT
		Driver * driver = new IocDriver(client, high, low, create);
		driver->setAutoclean(true);
		return (ummap_driver_t*)driver;
	#else
		UMMAP_FATAL("Ummap-io was build without support of IOC, cannot create the requested driver !");
		return NULL;
	#endif
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_c(const ummap_c_driver_t * driver, void * driver_data)
{
	Driver * cdriver = new CDriver(driver, driver_data);
	cdriver->setAutoclean(true);
	return (ummap_driver_t*)cdriver;
}

/*******************  FUNCTION  *********************/
void ummap_driver_destroy(ummap_driver_t * driver)
{
	assert(driver != NULL);
	Driver * driv = (Driver*)driver;
	delete driv;
}

/*******************  FUNCTION  *********************/
void ummap_driver_set_autoclean(ummap_driver_t * driver, bool autoclean)
{
	assert(driver != NULL);
	Driver * driv = (Driver*)(void*)driver;
	driv->setAutoclean(autoclean);
}

/*******************  FUNCTION  *********************/
void ummap_policy_group_register(const char * name, ummap_policy_t * policy)
{
	//check
	assert(getGlobalhandler() != NULL);
	assert(name != NULL);
	
	//reg
	Policy * pol = (Policy*)policy;
	getGlobalhandler()->registerPolicy(name, pol);
}

/*******************  FUNCTION  *********************/
void ummap_policy_group_destroy(const char * name)
{
	//check
	assert(getGlobalhandler() != NULL);
	assert(name != NULL);
	
	//unreg
	getGlobalhandler()->unregisterPolicy(name);
}

/*******************  FUNCTION  *********************/
ummap_policy_t * ummap_policy_create_fifo(size_t max_size, bool local)
{
	Policy * policy = new FifoPolicy(max_size, local);
	return (ummap_policy_t*)policy;
}

/*******************  FUNCTION  *********************/
ummap_policy_t * ummap_policy_create_fifo_window(size_t max_size, size_t window_size, bool local)
{
	Policy * policy = new FifoWindowPolicy(max_size, window_size, local);
	return (ummap_policy_t*)policy;
}

/*******************  FUNCTION  *********************/
ummap_policy_t * ummap_policy_create_lifo(size_t max_size, bool local)
{
	Policy * policy = new LifoPolicy(max_size, local);
	return (ummap_policy_t*)policy;
}

/*******************  FUNCTION  *********************/
size_t ummap_policy_get_memory(ummap_policy_t * policy)
{
	//nothing to do
	if (policy == NULL)
		return 0;
	
	//cast and call
	Policy * castedPolicy = (Policy*)policy;

	//ret
	return castedPolicy->getCurrentMemory();
}

/*******************  FUNCTION  *********************/
void umflush(void * ptr, size_t size, bool evict)
{
	getGlobalhandler()->flush(ptr, size, evict, false);
}

/*******************  FUNCTION  *********************/
void umsync(void * ptr, size_t size, bool evict)
{
	getGlobalhandler()->flush(ptr, size, evict, true);
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_get_driver(void * ptr)
{
	return (ummap_driver_t *)getGlobalhandler()->getDriver(ptr);
}

/*******************  FUNCTION  *********************/
void ummap_make_dirty(void * ptr, size_t size)
{
	getGlobalhandler()->makeDirty(ptr);
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_uri(const char * uri)
{
	Driver * driver = getGlobalhandler()->getUriHandler().buildDriver(uri);
	driver->setAutoclean(true);
	return (ummap_driver_t*)driver;
}

/*******************  FUNCTION  *********************/
ummap_policy_t * ummap_policy_create_uri(const char * uri, bool local)
{
	Policy * policy = getGlobalhandler()->getUriHandler().buildPolicy(uri, local);
	return (ummap_policy_t*)policy;
}

/*******************  FUNCTION  *********************/
void ummap_policy_destroy(ummap_policy_t * policy)
{
	//check
	assert(policy != NULL);

	//convert
	Driver * pol = (Driver*)policy;
	
	//destroy
	delete pol;
}

/*******************  FUNCTION  *********************/
void ummap_uri_set_variable(const char * name, const char * value)
{
	getGlobalhandler()->getUriHandler().registerVariable(name, value);
}

/*******************  FUNCTION  *********************/
void ummap_uri_set_variable_int(const char * name, int value)
{
	getGlobalhandler()->getUriHandler().registerVariable(name, value);
}

/*******************  FUNCTION  *********************/
void ummap_uri_set_variable_size_t(const char * name, size_t value)
{
	getGlobalhandler()->getUriHandler().registerVariable(name, value);
}

/*******************  FUNCTION  *********************/
void ummap_config_clovis_init_options(const char * ressource_file, int index)
{
	MeroRessource::setRessourceInfo(ressource_file, index);
}

/*******************  FUNCTION  *********************/
void ummap_config_ioc_init_options(const char * server, const char * port)
{
	IocRessource::setRessourceInfo(server, port);
}

/*******************  FUNCTION  *********************/
void ummap_mero_init(const char * ressourceFile, int ressourceIndex)
{
	getGlobalhandler()->initMero(ressourceFile, ressourceIndex);
}

/*******************  FUNCTION  *********************/
int ummap_cow_uri(void * addr, const char * uri, bool allow_exist)
{
	return getGlobalhandler()->getUriHandler().applyCow(addr, uri, allow_exist);
}

/*******************  FUNCTION  *********************/
int ummap_cow_clovis(void * addr, int64_t high, int64_t low, bool allow_exist)
{
	#if defined(HAVE_MERO) || defined(HAVE_MOTR)
		return getGlobalhandler()->applyCow<ClovisDriver>("Clovis", addr, [high, low, allow_exist](Mapping * mapping, ClovisDriver * driver){
			//create new driver
			ummap_driver_t * new_driver = ummap_driver_create_clovis(high, low, true);

			//copy data
			const size_t size = mapping->getStorageOffset() + mapping->getSize();
			mapping->copyToDriver((Driver*)new_driver, size);

			//apply
			driver->setObjectId(high, low);

			//clear
			ummap_driver_destroy(new_driver);

			//ok
			return 0;
		});
	#else
		UMMAP_FATAL("Try to cow a clovis driver, but ummap was compiled without mero/motr !");
		return -1;
	#endif
}

/*******************  FUNCTION  *********************/
int ummap_cow_fd(void * addr, int fd, bool allow_exist)
{
	//apply cow
	return getGlobalhandler()->applyCow<FDDriver>("FD", addr, [fd](Mapping * mapping, FDDriver * driver){
		//get file size
		//struct stat st;
		//int status = fstat(driver->getFd(), &st);
		//assumeArg(status == 0, "Fail to fstat the original file: %1").argStrErrno().end();
		//size_t origSize = st.st_size;

		//try clone
		FDDriver * typedDriver = dynamic_cast<FDDriver*>(driver);
		bool tryClone = typedDriver->cloneRange(fd, mapping->getStorageOffset(), mapping->getSize());

		//if not clone
		if (!tryClone) {
			//create driver
			FDDriver * newDriver = new FDDriver(fd);

			//copy to new driver
			mapping->copyToDriver(newDriver,mapping->getStorageOffset() + mapping->getSize());

			//clean
			delete newDriver;
		}

		//set fd
		driver->setFd(fd);

		//ok
		return 0;
	});
}

/*******************  FUNCTION  *********************/
int ummap_cow_fopen(void * addr, const char * file_path, const char * mode, bool allow_exist)
{
	//check exist
	if (allow_exist == false) {
		assumeArg(access( file_path, F_OK ) != 0, "Try to cow on an existing file (%1), but allow_exist is set to false !")
			.arg(file_path)
			.end();
	}

	//vars
	FILE * fp = NULL;
	int fd = -1;

	//if default value we use direct open and o_create
	if (*mode == '\0') {
		fd = open(file_path, O_RDWR | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		assumeArg(fd > 0, "Fail to open file '%1': %2").arg(file_path).argStrErrno().end();
	} else {
		//open
		fp = fopen(file_path, mode);
		assumeArg(fp != NULL, "Fail to open file '%1': %2").arg(file_path).argStrErrno().end();
		fd = fileno(fp);
	}

	//cow
	int status = ummap_cow_fd(addr, fd, allow_exist);

	//close
	if (*mode == '\0')
		close(fd);
	else
		fclose(fp);

	//ret
	return status;
}

/*******************  FUNCTION  *********************/
int ummap_cow_dax_fopen(void *addr, const char * file_path, const char * mode, bool allow_exist)
{
	//check exist
	if (allow_exist == false) {
		assumeArg(access( file_path, F_OK ) != 0, "Try to cow on an existing file (%1), but allow_exist is set to false !")
			.arg(file_path)
			.end();
	}

	//open
	FILE * fp = fopen(file_path, mode);
	assumeArg(fp != NULL, "Fail to open file '%1': %2")
		.arg(file_path)
		.argStrErrno()
		.end();

	//cow
	int fd = fileno(fp);
	int status = ummap_cow_dax(addr, fd, allow_exist);

	//close
	fclose(fp);

	//ret
	return status;
}

/*******************  FUNCTION  *********************/
int ummap_cow_dax(void * addr, int fd, bool allow_exist)
{
	//apply cow
	return getGlobalhandler()->applyCow<MmapDriver>("MMAP", addr, [fd](Mapping * mapping, MmapDriver * driver){
		//allocate a new driver
		MmapDriver * new_driver = new MmapDriver(fd, true);

		//map a new segment
		mapping->directMmapCow(new_driver);

		//replace in driver
		driver->setFd(fd);

		//delete temp driver
		delete new_driver;

		//ok
		return 0;
	});
}

/*******************  FUNCTION  *********************/
int ummap_cow_ioc(void * addr, int64_t high, int64_t low, bool allow_exist)
{
	#ifdef HAVE_IOC_CLIENT
		//call
		return getGlobalhandler()->applyCow<IocDriver>("IOC", addr, [high, low, allow_exist](Mapping * mapping, IocDriver * driver){
			return driver->cow(high, low, allow_exist, mapping->getStorageOffset(), mapping->getSize());
		});
	#else
		UMMAP_FATAL("Ummap-io was built without IOC support, cannot apply COW on the requested mapping !");
		return -1;
	#endif
}

/*******************  FUNCTION  *********************/
int ummap_switch_fopen(void * addr, const char * file_path, const char * mode, ummap_switch_clean_t clean_action)
{
	//open
	FILE * fp = fopen(file_path, mode);
	assumeArg(fp != NULL, "Fail to open file '%1': %2").arg(file_path).argStrErrno().end();

	//call
	int fd = fileno(fp);
	int status = getGlobalhandler()->applySwitch<FDDriver>("FD", addr, clean_action, [fd](FDDriver * driver){
		return driver->setFd(fd);
	});

	//close
	fclose(fp);

	//ok
	return status;
}

/*******************  FUNCTION  *********************/
int ummap_switch_fd(void * addr, int fd, ummap_switch_clean_t clean_action)
{
	//call
	return getGlobalhandler()->applySwitch<FDDriver>("FD", addr, clean_action, [fd](FDDriver * driver){
		return driver->setFd(fd);
	});
}

/*******************  FUNCTION  *********************/
int ummap_switch_dax(void * addr, int fd, ummap_switch_clean_t clean_action)
{
	return ummap_cow_dax(addr, fd, true);
}

/*******************  FUNCTION  *********************/
int ummap_switch_dax_fopen(void *addr, const char * file_path, const char * mode, ummap_switch_clean_t clean_action)
{
	return ummap_cow_dax_fopen(addr, file_path, mode, true);
}

/*******************  FUNCTION  *********************/
int ummap_switch_clovis(void * addr, int64_t high, int64_t low, ummap_switch_clean_t clean_action)
{
	#if defined(HAVE_MERO) || defined(HAVE_MOTR)
		//call
		return getGlobalhandler()->applySwitch<ClovisDriver>("IOC", addr, clean_action, [high, low](ClovisDriver * driver){
			return driver->setObjectId(high, low);
		});
	#else
		UMMAP_FATAL("Ummap-io was built without Mero/Motr support, cannot apply COW on the requested mapping !");
		return -1;
	#endif
}

/*******************  FUNCTION  *********************/
int ummap_switch_ioc(void * addr, int64_t high, int64_t low, ummap_switch_clean_t clean_action)
{
	#ifdef HAVE_IOC_CLIENT
		//call
		return getGlobalhandler()->applySwitch<IocDriver>("IOC", addr, clean_action, [high, low](IocDriver * driver){
			return driver->switchDestination(high, low);
		});
	#else
		UMMAP_FATAL("Ummap-io was built without IOC support, cannot apply COW on the requested mapping !");
		return -1;
	#endif
}

/*******************  FUNCTION  *********************/
int ummap_switch_uri(void * addr, const char * uri, ummap_switch_clean_t clean_action)
{
	return getGlobalhandler()->getUriHandler().applySwitch(addr, uri, clean_action);
}

/*******************  FUNCTION  *********************/
ummap_quota_t * ummap_quota_create_local(size_t max_memory)
{
	ummap_quota_t * res = (ummap_quota_t *)new PolicyQuotaLocal(max_memory);
	return res;
}

/*******************  FUNCTION  *********************/
ummap_quota_t * ummap_quota_create_inter_proc(const char * group_name, size_t max_memory)
{
	ummap_quota_t * res = (ummap_quota_t *)new PolicyQuotaInterProc(group_name, max_memory);
	return res;
}

/*******************  FUNCTION  *********************/
ummap_quota_t * ummap_quota_create_inter_proc_env(const char * group_name, const char * env_name, size_t default_max_mem)
{
	//check
	assume(env_name != NULL && env_name[0] != '\0', "Invalid NULL or empty env name !");

	//set to default first
	size_t value = default_max_mem;

	//get env
	char * var = getenv(env_name);
	if (var != NULL) {
		value = fromHumanMemSize(var);
	}

	//build
	if (value == 0)
		return NULL;
	else
		return ummap_quota_create_inter_proc(group_name, value);
}

/*******************  FUNCTION  *********************/
void ummap_quota_destroy(ummap_quota_t * quota)
{
	if (quota != NULL)
		delete (PolicyQuotaInterProc*)quota;
}

/*******************  FUNCTION  *********************/
void ummap_quota_register_policy(ummap_quota_t * quota, ummap_policy_t * policy)
{
	//nothing to do
	if (quota == NULL || policy == NULL)
		return;
	
	//cast
	Policy * castedPolicy = (Policy*)policy;
	PolicyQuota * castedQuota = (PolicyQuota*)quota;

	//call
	castedQuota->registerPolicy(castedPolicy);
}

/*******************  FUNCTION  *********************/
void ummap_quota_unregister_policy(ummap_quota_t * quota, ummap_policy_t * policy)
{
	//nothing to do
	if (quota == NULL || policy == NULL)
		return;
	
	//cast
	Policy * castedPolicy = (Policy*)policy;
	PolicyQuota * castedQuota = (PolicyQuota*)quota;

	//call
	castedQuota->unregisterPolicy(castedPolicy);
}
