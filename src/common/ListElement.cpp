/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstdlib>
#include <cassert>
//internal
#include "ListElement.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummap;

/*******************  FUNCTION  *********************/
ListElement::ListElement(void)
{
	this->next = this->prev = this;
}

/*******************  FUNCTION  *********************/
ListElement::~ListElement(void)
{
	this->removeFromList();
}

/*******************  FUNCTION  *********************/
void ListElement::removeFromList(void)
{
	//remove
	this->next->prev = this->prev;
	this->prev->next = this->next;
	//reset
	this->next = this->prev = this;
}

/*******************  FUNCTION  *********************/
bool ListElement::isAlone(void)
{
	return (this->next == this && this->prev == this);
}

/*******************  FUNCTION  *********************/
bool ListElement::isInList(void)
{
	return (this->next != this && this->prev != this);
}

/*******************  FUNCTION  *********************/
void ListElement::insertAfter(ListElement & element)
{
	//check
	assert(element.isAlone());

	//setup element
	element.next = this->next;
	element.prev = this;

	//change next
	this->next->prev = &element;

	//change current
	this->next = &element;
}

/*******************  FUNCTION  *********************/
void ListElement::insertBefore(ListElement & element)
{
	//check
	assert(element.isAlone());

	//setup element
	element.prev = this->prev;
	element.next = this;

	//change next
	this->prev->next = &element;

	//change current
	this->prev = &element;
}

/*******************  FUNCTION  *********************/
ListElement * ListElement::popNext(void)
{
	if (isAlone()) {
		return NULL;
	} else {
		ListElement * next = this->next;
		next->removeFromList();
		return next;
	}
}

/*******************  FUNCTION  *********************/
ListElement * ListElement::popPrev(void)
{
	if (isAlone()) {
		return NULL;
	} else {
		ListElement * prev = this->prev;
		prev->removeFromList();
		return prev;
	}
}
