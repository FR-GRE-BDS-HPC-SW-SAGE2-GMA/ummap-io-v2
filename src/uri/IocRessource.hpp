/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_IOC_RESSOURCE_HPP
#define UMMAP_IOC_RESSOURCE_HPP

/********************  HEADERS  *********************/
#include <string>
#ifdef HAVE_IOC_CLIENT
	#include <ioc-client.h>
#endif //HAVE_IOC_CLIENT
#include "RessourceHandler.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
class IocRessource : public Ressource
{
	public:
		IocRessource(void);
		virtual ~IocRessource(void);
		static void setRessourceInfo(const std::string & server, const std::string & port);
		#ifdef HAVE_IOC_CLIENT
			static struct ioc_client_t * getClient(void);
		#endif
	private:
		static std::string server;
		static std::string port;
		#ifdef HAVE_IOC_CLIENT
			static struct ioc_client_t * client;
		#endif //HAVE_IOC_CLIENT
};

}

#endif //UMMAP_IOC_RESSOURCE_HPP
