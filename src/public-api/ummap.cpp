/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cerrno>
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
void * ummap(size_t size, size_t segment_size, size_t storage_offset, int protection, int flags, ummap_driver_t * driver, ummap_policy_t * local_policy, const char * policy_group)
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
	return getGlobalhandler()->ummap(size, segment_size, storage_offset, protection, flags, driv, pol, policy_group_checked);
}

/*******************  FUNCTION  *********************/
int umunmap(void * ptr, int sync)
{
	//check
	assert(getGlobalhandler() != NULL);
	assert(ptr != NULL);

	//call
	return getGlobalhandler()->umunmap(ptr, sync);
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
#ifdef HAVE_IOC_CLIENT
	ummap_driver_t * ummap_driver_create_ioc(struct ioc_client_t * client, int64_t high, int64_t low)
	{
		Driver * driver = new IocDriver(client, high, low);
		driver->setAutoclean(true);
		return (ummap_driver_t*)driver;
	}
#endif

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
ummap_policy_t * ummap_policy_create_lifo(size_t max_size, bool local)
{
	Policy * policy = new LifoPolicy(max_size, local);
	return (ummap_policy_t*)policy;
}

/*******************  FUNCTION  *********************/
void umsync(void * ptr, size_t size, int evict)
{
	getGlobalhandler()->flush(ptr, size, evict);
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_uri(const char * uri)
{
	Driver * driver = getGlobalhandler()->getUriHandler().buildDriver(uri);
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
