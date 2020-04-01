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
using namespace ummapio;

/*******************  FUNCTION  *********************/
/**
 * ListElement constructor, it init the prev/next by pointing
 * the current element to close the loop.
**/
ListElement::ListElement(void)
{
	this->next = this->prev = this;
}

/*******************  FUNCTION  *********************/
/**
 * ListElement destructor, it automatically remove the element
 * from the list if it is in a list.
**/
ListElement::~ListElement(void)
{
	this->removeFromList();
}

/*******************  FUNCTION  *********************/
/**
 * Remove the current element from the list and close the loop
 * by self pointing next/prev.
**/
void ListElement::removeFromList(void)
{
	//remove
	this->next->prev = this->prev;
	this->prev->next = this->next;
	//reset
	this->next = this->prev = this;
}

/*******************  FUNCTION  *********************/
/**
 * Check if the element is alons (not in a list).
 * @return Return true is not in a list, false otherwise.
**/
bool ListElement::isAlone(void)
{
	return (this->next == this && this->prev == this);
}

/*******************  FUNCTION  *********************/
/**
 * Check if the element is in a list.
 * @return True if in a list, false otherwise.
**/
bool ListElement::isInList(void)
{
	return (this->next != this && this->prev != this);
}

/*******************  FUNCTION  *********************/
/**
 * Insert the given element after the current one.
 * @param element The element to insert after the current one.
**/
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
/**
 * Insert the given element before the current one.
 * @param element The element to insert before the current one.
**/
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
/**
 * Remove the next element from the list and return it as pointer.
 * @return Return the next element which has been removed from the list.
**/
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
/**
 * Remove the previous element from the list and return it as pointer.
 * @return Return the previous element which has been removed from the list.
**/
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
