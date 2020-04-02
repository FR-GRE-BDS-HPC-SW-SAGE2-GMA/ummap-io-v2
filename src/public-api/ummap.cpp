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
#include "../common/Debug.hpp"
#include "../common/URI.hpp"
#include "../core/GlobalHandler.hpp"
#include "../drivers/FDDriver.hpp"
#include "../drivers/MemoryDriver.hpp"
#include "../drivers/DummyDriver.hpp"
#include "../policies/FifoPolicy.hpp"
#include "ummap.h"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
void ummap_init(void)
{
	ummapio::GlobalHandler * handler = new ummapio::GlobalHandler();
	ummapio::setGlobalHandler(handler);
	ummapio::setupSegfaultHandler();
}

/*******************  FUNCTION  *********************/
void ummap_finalize(void)
{
	ummapio::unsetSegfaultHandler();
	ummapio::clearGlobalHandler();
}

/*******************  FUNCTION  *********************/
void * ummap(size_t size, size_t segment_size, size_t storage_offset, ummap_mapping_prot_t protection, ummap_driver_t * driver, ummap_policy_t * local_policy, const char * policy_group)
{
	//check
	assert(getGlobalhandler() != NULL);

	//convert
	MappingProtection prot = (MappingProtection)protection;
	Driver * driv = (Driver*)(driver);
	Policy * pol = (Policy*)(local_policy);

	//call & ret
	return getGlobalhandler()->ummap(size, segment_size, storage_offset, prot, driv, pol, policy_group);
}

/*******************  FUNCTION  *********************/
int umunmap(void * ptr)
{
	//check
	assert(getGlobalhandler() != NULL);
	assert(ptr != NULL);

	//call
	return getGlobalhandler()->umunmap(ptr);
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
	return (ummap_driver_t*)driver;
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_fopen(const char * file_path, const char * mode)
{
	//open
	FILE * fp = fopen(file_path, mode);
	assumeArg(fp != NULL, "Fail to open file '%1': %2")
		.arg(file_path)
		.arg(strerror(errno))
		.end();
	
	//create driver
	ummap_driver_t * res = ummap_driver_create_fd(fileno(fp));

	//close
	fclose(fp);

	//return
	return res;
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_fd(int fd)
{
	Driver * driver = new FDDriver(fd);
	return (ummap_driver_t*)driver;
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_memory(size_t size)
{
	Driver * driver = new MemoryDriver(size);
	return (ummap_driver_t*)driver;
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
ummap_policy_t * umamp_policy_create_fifo(size_t max_size, bool local)
{
	Policy * policy = new FifoPolicy(max_size, local);
	return (ummap_policy_t*)policy;
}

/*******************  FUNCTION  *********************/
void ummap_flush(void * ptr, size_t size)
{
	getGlobalhandler()->flush(ptr, size);
}

/*******************  FUNCTION  *********************/
ummap_driver_t * ummap_driver_create_uri(const char * uri)
{
	//check
	assert(uri != NULL);
	URI parser(uri);

	//cases
	std::string type = parser.getType();
	if (type == "file") {
		return ummap_driver_create_fopen(parser.getPath().c_str(), parser.getParam("mode").c_str());
	} else if (type == "mem") {
		return ummap_driver_create_memory(atol(parser.getPath().c_str()));
	} else if (type == "dummy") {
		return ummap_driver_create_dummy(atol(parser.getPath().c_str()));
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to build driver : %1").arg(uri).end();
		return NULL;
	}
}

/*******************  FUNCTION  *********************/
ummap_policy_t * umamp_policy_create_uri(const char * uri, bool local)
{
	//check
	assert(uri != NULL);
	URI parser(uri);

	//cases
	std::string type = parser.getType();
	if (type == "fifo") {
		return umamp_policy_create_fifo(atol(parser.getPath().c_str()), local);
	} else {
		UMMAP_FATAL_ARG("Invalid ressource type to build policy : %1").arg(uri).end();
		return NULL;
	}
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
