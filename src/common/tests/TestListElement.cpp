/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../ListElement.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/*******************  FUNCTION  *********************/
TEST(TestListElement, constructor)
{
	//create
	ListElement element;

	//check status
	ASSERT_EQ(&element, &element.getNext());
	ASSERT_EQ(&element, &element.getPrev());
}

/*******************  FUNCTION  *********************/
TEST(TestListElement, isAlone)
{
	//create
	ListElement e1;
	ListElement e2;

	//test default
	ASSERT_TRUE(e1.isAlone());
	
	//insert and test
	e1.insertAfter(e2);
	ASSERT_FALSE(e1.isAlone());
}

/*******************  FUNCTION  *********************/
TEST(TestListElement, isInList)
{
	//create
	ListElement e1;
	ListElement e2;

	//test default
	ASSERT_FALSE(e1.isInList());
	ASSERT_FALSE(e2.isInList());
	
	//insert and test
	e1.insertAfter(e2);
	ASSERT_TRUE(e1.isInList());
	ASSERT_TRUE(e2.isInList());
}

/*******************  FUNCTION  *********************/
TEST(TestListElement, insertAfter)
{
	//create
	ListElement e1;
	ListElement e2;
	ListElement e3;

	//insert
	e1.insertAfter(e2);
	e2.insertAfter(e3);

	//check
	EXPECT_EQ(&e3, &e1.getPrev());
	EXPECT_EQ(&e2, &e1.getNext());
}

/*******************  FUNCTION  *********************/
TEST(TestListElement, insertBefore)
{
	//create
	ListElement e1;
	ListElement e2;
	ListElement e3;

	//insert
	e1.insertBefore(e2);
	e2.insertBefore(e3);

	//check
	EXPECT_EQ(&e2, &e1.getPrev());
	EXPECT_EQ(&e3, &e1.getNext());
}


/*******************  FUNCTION  *********************/
TEST(TestListElement, popNext)
{
	//create
	ListElement e1;
	ListElement e2;
	ListElement e3;

	//insert
	e1.insertAfter(e2);
	e2.insertAfter(e3);

	//pop
	ListElement * pop1 = e1.popNext();
	ASSERT_EQ(&e2, pop1);

	ListElement * pop2 = e1.popNext();
	ASSERT_EQ(&e3, pop2);

	ListElement * pop3 = e1.popNext();
	ASSERT_EQ(nullptr, pop3);
}

/*******************  FUNCTION  *********************/
TEST(TestListElement, popPrev)
{
	//create
	ListElement e1;
	ListElement e2;
	ListElement e3;

	//insert
	e1.insertBefore(e2);
	e2.insertBefore(e3);

	//pop
	ListElement * pop1 = e1.popPrev();
	ASSERT_EQ(&e2, pop1);

	ListElement * pop2 = e1.popPrev();
	ASSERT_EQ(&e3, pop2);

	ListElement * pop3 = e1.popPrev();
	ASSERT_EQ(nullptr, pop3);
}
