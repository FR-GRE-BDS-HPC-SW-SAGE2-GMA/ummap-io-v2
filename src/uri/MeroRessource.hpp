/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_MERO_RESSOURCE_HPP
#define UMMAP_MERO_RESSOURCE_HPP

/********************  HEADERS  *********************/
#include <string>
#include "config.h"
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
		static void setRessourceInfo(const std::string & ressourceFile, int ressourceIndex);
	private:
		#if defined(HAVE_MERO) || defined(HAVE_MOTR)
			bool hasInit;
		#endif
		static std::string ressourceFile;
		static int ressourceIndex;
};

}

#endif //UMMAP_MERO_RESSOURCE_HPP
