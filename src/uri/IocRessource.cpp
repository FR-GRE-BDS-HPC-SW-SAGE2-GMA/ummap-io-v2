/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <cstdio>
#include "../common/Debug.hpp"
#include "../core/GlobalHandler.hpp"
#include "IocRessource.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  GLOBALS  **********************/
std::string IocRessource::server = "127.0.0.1";
std::string IocRessource::port = "8556";
#ifdef HAVE_IOC_CLIENT
	struct ioc_client_t * IocRessource::client = NULL;
#endif //HAVE_IOC_CLIENT

/*******************  FUNCTION  *********************/
IocRessource::IocRessource(void)
{
	#ifdef HAVE_IOC_CLIENT
		if (this->client == NULL)
		{
			this->client = ioc_client_init(server.c_str(), port.c_str());
			assume(this->client != NULL, "Failed to init IOC API !");
		}
	#endif
}

/*******************  FUNCTION  *********************/
#ifdef HAVE_IOC_CLIENT
struct ioc_client_t * IocRessource::getClient(void)
{
	return client;
}
#endif

/*******************  FUNCTION  *********************/
IocRessource::~IocRessource(void)
{
	#ifdef HAVE_IOC_CLIENT
		if (client != NULL)
		{
			ioc_client_fini(client);
			client = NULL;
		}
	#endif
}

/*******************  FUNCTION  *********************/
void IocRessource::setRessourceInfo(const std::string & server, const std::string & port)
{
	IocRessource::server = server;
	IocRessource::port = port;
}
