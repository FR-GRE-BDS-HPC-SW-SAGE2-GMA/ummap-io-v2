/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../RessourceHandler.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/********************  CONSTS  **********************/
class RessourceMock : public Ressource
{
	public:
		RessourceMock(void) {ressourceId++;};
		~RessourceMock(void) {ressourceId--;};
		static int ressourceId;
};

/*******************  GLOBALS  **********************/
int RessourceMock::ressourceId = 0;

/*******************  FUNCTION  *********************/
TEST(TestRessourceHandler, constructor)
{
	RessourceHandler handler;
}

/*******************  FUNCTION  *********************/
TEST(TestRessourceHandler, checkRessource)
{
	//check status
	ASSERT_EQ(0, RessourceMock::ressourceId);

	//create
	RessourceHandler * handler = new RessourceHandler;
	handler->checkRessource<RessourceMock>("mock");
	ASSERT_EQ(1, RessourceMock::ressourceId);

	//destroy
	delete handler;
	ASSERT_EQ(0, RessourceMock::ressourceId);
}
