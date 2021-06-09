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
#if defined(HAVE_MERO) || defined(HAVE_MOTR)
	#include "clovis_api.h"
#endif // HAVE_MERO || HAVE_MOTR

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  GLOBALS  **********************/
std::string MeroRessource::ressourceFile = "mero_ressource_file.rc";
int MeroRessource::ressourceIndex = 0;

/*******************  FUNCTION  *********************/
MeroRessource::MeroRessource(void)
{
	#if defined(HAVE_MERO)
		if (clovis_instance == NULL)
		{
			int res = c0appz_init(ressourceIndex, (char*)ressourceFile.c_str());
			assume(res == 0, "Failed to init mero/clovis API !");
			setupSegfaultHandler();
			this->hasInit = true;
		} else {
			this->hasInit = false;
		}
	#elif defined(HAVE_MOTR)
		if (clovis_instance == NULL)
		{
			c0appz_set_manual_rc((char*)ressourceFile.c_str());
			int res = c0appz_init(ressourceIndex);
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
	#if defined(HAVE_MERO) || defined(HAVE_MOTR)
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
