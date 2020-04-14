/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_MERO_RESSOURCE_HPP
#define UMMAP_MERO_RESSOURCE_HPP

/********************  HEADERS  *********************/
#include "RessourceHandler.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
class MeroRessource : public Ressource
{
	public:
		MeroRessource(void);
		virtual ~MeroRessource(void);
};

}

#endif //UMMAP_MERO_RESSOURCE_HPP