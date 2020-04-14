/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <cstdio>
#include "MeroRessource.hpp"
#ifdef MERO_FOUND
	#include "../public-api/clovis_api.h"
#endif // MERO_FOUND

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  GLOBALS  **********************/
std::string MeroRessource::ressourceFile = "mero_ressource_file.rc";
int MeroRessource::ressourceIndex = 0;

/*******************  FUNCTION  *********************/
MeroRessource::MeroRessource(void)
{
	#ifdef MERO_FOUND
		if (clovis_instance == NULL)
		{
			int res = c0appz_init(ressourceIndex, ressourceFile.c_str());
			assume(res == 0, "Failed to init mero/clovis API !");
			this->hasInit = true;
		} else {
			this->hasInit = false;
		}
	#endif
}

/*******************  FUNCTION  *********************/
MeroRessource::~MeroRessource(void)
{
	#ifdef MERO_FOUND
		if (this->hasInit) {
			c0appz_free();
			this->hasInit = false;
		}
	#endif
}

/*******************  FUNCTION  *********************/
void MeroRessource::setRessourceInfo(int ressourceIndex, const std::string & ressourceFile)
{
	MeroRessource::ressourceIndex = ressourceIndex;
	MeroRessource::ressourceFile = ressourceFile;
}
