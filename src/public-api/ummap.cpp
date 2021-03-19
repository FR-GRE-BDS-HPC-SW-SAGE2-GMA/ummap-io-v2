/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
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
//local
#include <config.h>
#include "../common/Debug.hpp"
#include "../core/GlobalHandler.hpp"
#include "../uri/MeroRessource.hpp"
#include "../uri/IocRessource.hpp"
#include "../drivers/FDDriver.hpp"
#include "../drivers/MemoryDriver.hpp"
#include "../drivers/DummyDriver.hpp"
#include "../drivers/MmapDriver.hpp"
#include "../drivers/CDriver.hpp"
#ifdef HAVE_MERO
	#include "../drivers/ClovisDriver.hpp"
	#include "clovis_api.h"
#endif
#ifdef HAVE_IOC_CLIENT
	#include "../drivers/IocDriver.hpp"
#endif
#include "../policies/FifoPolicy.hpp"
#include "../policies/FifoWindowPolicy.hpp"
#include "../policies/LifoPolicy.hpp"
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
	assumeArg(fp != NULL, "Fail to open file '%1': %2")
		.arg(file_path)
		.argStrErrno()
		.end();
	
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
	assumeArg(fp != NULL, "Fail to open file '%1': %2")
		.arg(file_path)
		.argStrErrno()
		.end();
	
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
	#ifdef HAVE_MERO
		struct m0_uint128 object_id = {high, low};
		ClovisDriver * driver = new ClovisDriver(object_id, create);
		driver->setAutoclean(true);
		return (ummap_driver_t*)driver;
	#else
		UMMAP_FATAL("Try to create a clovis driver, but ummap was compiled without mero !");
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
void umsync(void * ptr, size_t size, bool evict)
{
	getGlobalhandler()->flush(ptr, size, evict);
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
int ummap_cow_uri(void * addr, const char * uri, bool allow_exist)
{
	return getGlobalhandler()->getUriHandler().applyCow(addr, uri, allow_exist);
}

/*******************  FUNCTION  *********************/
int ummap_cow_fd(void * addr, int fd, bool allow_exist)
{
	//apply cow
	return getGlobalhandler()->applyCow<FDDriver>("FD", addr, [fd](Mapping * mapping, FDDriver * driver){
		//get file size
		struct stat st;
		int status = fstat(driver->getFd(), &st);
		assumeArg(status == 0, "Fail to fstat the original file: %1").argStrErrno().end();
		size_t origSize = st.st_size;

		//create driver
		FDDriver * newDriver = new FDDriver(fd);

		//copy to new driver
		mapping->copyToDriver(newDriver,origSize);

		//set fd
		driver->setFd(fd);

		//clear
		delete newDriver;

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

	//open
	FILE * fp = fopen(file_path, mode);
	assumeArg(fp != NULL, "Fail to open file '%1': %2")
		.arg(file_path)
		.argStrErrno()
		.end();

	//cow
	int fd = fileno(fp);
	int status = ummap_cow_fd(addr, fd, allow_exist);

	//close
	fclose(fp);

	//ret
	return status;
}

/*******************  FUNCTION  *********************/
int ummap_cow_ioc(void * addr, int64_t high, int64_t low, bool allow_exist)
{
	#ifdef HAVE_IOC_CLIENT
		//call
		return getGlobalhandler()->applyCow<IocDriver>("IOC", addr, [high, low, allow_exist](Mapping * mapping, IocDriver * driver){
			return driver->cow(high, low, allow_exist);
		});
	#else
		UMMAP_FATAL("Ummap-io was built without IOC support, cannot apply COW on the requested mapping !");
		return -1;
	#endif
}

/*******************  FUNCTION  *********************/
int ummap_switch_ioc(void * addr, int64_t high, int64_t low, bool drop_clean)
{
	#ifdef HAVE_IOC_CLIENT
		//call
		return getGlobalhandler()->applySwitch<IocDriver>("IOC", addr, drop_clean, [high, low](IocDriver * driver){
			return driver->switchDestination(high, low);
		});
	#else
		UMMAP_FATAL("Ummap-io was built without IOC support, cannot apply COW on the requested mapping !");
		return -1;
	#endif
}

/*******************  FUNCTION  *********************/
int ummap_switch_uri(void * addr, const char * uri, bool drop_clean)
{
	return getGlobalhandler()->getUriHandler().applySwitch(addr, uri, drop_clean);
}
