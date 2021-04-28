/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <cstdio>
#include "config.h"
#include "../common/Debug.hpp"
#include "../core/GlobalHandler.hpp"
#include "MeroRessource.hpp"
#ifdef HAVE_MERO
	#include "../public-api/clovis_api.h"
#endif // HAVE_MERO

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  GLOBALS  **********************/
std::string MeroRessource::ressourceFile = "mero_ressource_file.rc";
int MeroRessource::ressourceIndex = 0;

/*******************  FUNCTION  *********************/
MeroRessource::MeroRessource(void)
{
	#ifdef HAVE_MERO
		if (clovis_instance == NULL)
		{
			int res = c0appz_init(ressourceIndex, (char*)ressourceFile.c_str());
			assume(res == 0, "Failed to init mero/clovis API !");
			setupSegfaultHandler();
			this->hasInit = true;
		} else {
			this->hasInit = false;
		}
	#endif
}

/*******************  FUNCTION  *********************/
MeroRessource::~MeroRessource(void)
{
	#ifdef HAVE_MERO
		if (this->hasInit) {
			c0appz_free();
			this->hasInit = false;
		}
	#endif
}

/*******************  FUNCTION  *********************/
void MeroRessource::setRessourceInfo(const std::string & ressourceFile, int ressourceIndex)
{
	MeroRessource::ressourceIndex = ressourceIndex;
	MeroRessource::ressourceFile = ressourceFile;
}
