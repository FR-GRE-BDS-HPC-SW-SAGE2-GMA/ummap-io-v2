/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_LIST_ELEMENT_HPP
#define UMMAP_LIST_ELEMENT_HPP

/********************  HEADERS  *********************/

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
/**
 * Define a listable element with prev/next pointers.
 * This is used to build the list for FIFO/LIFO policies.
**/
class ListElement
{
	public:
		ListElement(void);
		~ListElement(void);
		void removeFromList(void);
		bool isAlone(void);
		bool isInList(void);
		ListElement & getPrev(void) {return *this->prev;};
		ListElement & getNext(void) {return *this->next;};
		void insertAfter(ListElement & element);
		void insertBefore(ListElement & element);
		ListElement * popNext(void);
		ListElement * popPrev(void);
	protected:
		/** Pointer to previous element **/
		ListElement * prev;
		/** Pointer to next element **/
		ListElement * next;
};

}

#endif //UMMAP_LIST_ELEMENT_HPP
