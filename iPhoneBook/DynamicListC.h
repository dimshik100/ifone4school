/*--------------------------------------------------------------
   DynamicListC.h -	An implementation of a simple doubly-linked
					list written in classic C.
					(c) Arthur Liberman, 2009
  --------------------------------------------------------------*/

/*
	Dima and Arthur's comment:
	This code was written by Arthur Liberman.
	A variation of it was used in one of our Algorithms tasks year 1 semester 2.
	We're reusing it in this project for a nice linked list implementation.
*/

// Makes sure we don't get any redefinition errors.
#pragma once

// Define a boolean type - "bool".
#ifndef bool
	typedef enum { false, true } bool;
#endif

// Define our type.		
typedef void* ListType;

// Define a List node struct
// Recoursive definition of a struct.
typedef struct _ListNode *ListNode;
struct _ListNode
{
	ListType type;
	ListNode pNext;
	ListNode pPrevious;
};

// Define the "DynamicListC" object.
typedef struct _DynamicListC
{
	int itemCount;
	ListNode pStart;
	ListNode pEnd;
	ListNode pCurrent;
}*DynamicListC;

// Initialize the linked list
bool listInit(DynamicListC *pList);

// Gets the number of items in the list
int listGetListCount(DynamicListC pList);

// Select the first item in the linked list
ListNode listSelectFirst(DynamicListC pList);

// Select the last item in the linked list
ListNode listSelectLast(DynamicListC pList);

// Returns the currently selected item from the linked list
ListNode listSelectCurrent(DynamicListC pList);

// Select the following item from the current item in the linked list
ListNode listSelectNext(DynamicListC pList, ListNode pNode);

// Select the preceeding item from the current item in the linked list
ListNode listSelectPrevious(DynamicListC pList, ListNode pNode);

// Retrieves the value in the selected node.
bool listGetValue(DynamicListC pList, ListNode pNode, ListType *value);

// Insert a new item after a node
bool listInsertAfter(DynamicListC pList, const ListType *value, ListNode pNode);

// Insert a new item before a node
bool listInsertBefore(DynamicListC pList, const ListType *value, ListNode pNode);

// Insert a new item after the last node
bool listInsertAfterEnd(DynamicListC pList, const ListType *value);

// Insert a new item before the first node
bool listInsertBeforeStart(DynamicListC pList, const ListType *value);

// Delete a node from the linked list
void listDeleteNode(DynamicListC pList, ListNode pNode);

// Delete the list and free the memory.
void listFree(DynamicListC *pList);