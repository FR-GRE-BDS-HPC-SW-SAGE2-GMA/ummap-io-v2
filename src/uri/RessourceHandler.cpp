/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include "RessourceHandler.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
RessourceHandler::RessourceHandler(void)
{

}

/*******************  FUNCTION  *********************/
RessourceHandler::~RessourceHandler(void)
{
	//CRITICAL SECTION
	{
		//lock
		std::lock_guard<std::mutex> lockGuard(this->mutex);

		//loop all & destroy
		for (auto it : this->ressourceMap) {
			delete it.second;
		}
	}
}
