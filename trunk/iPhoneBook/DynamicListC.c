/*--------------------------------------------------------------
   DynamicListC.c -	An implementation of a simple doubly-linked
					list written in classic C.
					(c) Arthur Liberman, 2009
  --------------------------------------------------------------*/

#include "stdafx.h" // May not be required, depends on the type of project you create.
#include "DynamicListC.h"
#include <stdio.h>
#include <malloc.h>

void listInitListParams(DynamicListC pList, ListNode pNode);

// Initialize the linked list
bool listInit(DynamicListC *pList)
{
	*pList = (DynamicListC)calloc(1, sizeof(struct _DynamicListC));
	return (pList != NULL);
}

// Gets the number of items in the list
int listGetListCount(DynamicListC pList)
{
	return pList->itemCount;
}

// Initialize linked list's parameters - used internally only!
void listInitListParams(DynamicListC pList, ListNode pNode)
{
	pList->pStart = pList->pEnd = pList->pCurrent = pNode;
}

// Select the first item in the linked list
ListNode listSelectFirst(DynamicListC pList)
{
	pList->pCurrent = pList->pStart;
	return pList->pStart;
}

// Select the last item in the linked list
ListNode listSelectLast(DynamicListC pList)
{
	pList->pCurrent = pList->pEnd;
	return pList->pEnd;
}

// Returns the currently selected item from the linked list
ListNode listSelectCurrent(DynamicListC pList)
{
	return pList->pCurrent;
}

// Select the following item from the current item in the linked list
ListNode listSelectNext(DynamicListC pList, ListNode pNode)
{
	ListNode pNew = NULL;

	// Decide wether to use the provided node or the currently selected one.
	if (pNode)
		pNew = pNode->pNext;
	else
	{
		// Make sure the list is not empty or that we're at the end.
		if (pList->pCurrent)
		{
			pList->pCurrent = pList->pCurrent->pNext;
			pNew = pList->pCurrent;
		}
	}

	return pNew;
}

// Select the preceeding item from the current item in the linked list
ListNode listSelectPrevious(DynamicListC pList, ListNode pNode)
{
	ListNode pNew = NULL;

	// Decide wether to use the provided node or the currently selected one.
	if (pNode)
		pNew = pNode->pPrevious;
	else
	{
		// Make sure the list is not empty or that we're at the end.
		if (pList->pCurrent)
		{
			pList->pCurrent = pList->pCurrent->pPrevious;
			pNew = pList->pCurrent;
		}
	}

	return pNew;
}

// Retrieves the value in the selected node.
bool listGetValue(DynamicListC pList, ListNode pNode, ListType *value)
{
	bool ret = false;
	ListNode pCur;

	// Decide wether to use the provided node or the currently selected one.
	pCur = (pNode) ? pNode : listSelectCurrent(pList);

	if (pCur)
	{
		*value = pCur->type;
		ret = true;
	}
	
	return ret;
}

// Insert a new item after a node
bool listInsertAfter(DynamicListC pList, const ListType *value, ListNode pNode)
{
	ListNode pCur;
	ListNode pNew = (ListNode)calloc(1, sizeof(*pNew));

	// Decide wether to use the provided node or the currently selected one.
	pCur = (pNode) ? pNode : listSelectCurrent(pList);

	if (pNew)
	{
		pNew->type = *value;
		// If list is already instantiated
		if (pList->pStart)
		{
			pNew->pNext = pCur->pNext;
			pNew->pPrevious = pCur;
			if (pCur->pNext)
				pCur->pNext->pPrevious = pNew;
			pCur->pNext = pNew;
			if (!pNew->pNext)
				pList->pEnd = pNew;
		}
		else
			listInitListParams(pList, pNew);
		
		pList->itemCount++;
	}

	return (pNew != NULL);
}

// Insert a new item before a node
bool listInsertBefore(DynamicListC pList, const ListType *value, ListNode pNode)
{
	ListNode pCur;
	ListNode pNew = (ListNode)calloc(1, sizeof(*pNew));

	// Decide wether to use the provided node or the currently selected one.
	pCur = (pNode) ? pNode : listSelectCurrent(pList);

	if (pNew)
	{
		pNew->type = *value;
		// If list is already instantiated
		if (pList->pStart)
		{
			pNew->pNext = pCur;
			pNew->pPrevious = pCur->pPrevious;
			if (pCur->pPrevious)
				pCur->pPrevious->pNext = pNew;
			pCur->pPrevious = pNew;
			if (!pNew->pPrevious)
				pList->pStart = pNew;

		}
		else
			listInitListParams(pList, pNew);
		
		pList->itemCount++;
	}

	return (pNew != NULL);
}

// Insert a new item after the last node
bool listInsertAfterEnd(DynamicListC pList, const ListType *value)
{
	return listInsertAfter(pList, value, pList->pEnd);
}

// Insert a new item before the first node
bool listInsertBeforeStart(DynamicListC pList, const ListType *value)
{
	return listInsertAfter(pList, value, pList->pStart);
}

// Delete a node from the linked list
void listDeleteNode(DynamicListC pList, ListNode pNode)
{
	if (pNode)
	{
		// If pNode is at the start of the list
		if (pNode == pList->pStart)
		{
			if (listSelectCurrent(pList) == pNode)
				listSelectNext(pList, NULL);
			pList->pStart = pList->pStart->pNext;
		}
		// If pNode is at the end of the list
		else if (pNode == pList->pEnd)
		{
			if (listSelectCurrent(pList) == pNode)
				listSelectPrevious(pList, NULL);
			pList->pEnd = pList->pEnd->pPrevious;
		}
		// If pNode is somewhere in the middle
		else
		{
			if (listSelectCurrent(pList) == pNode)
				listSelectNext(pList, NULL);
			if (pNode->pPrevious)
				pNode->pPrevious->pNext = pNode->pNext;
			if (pNode->pNext)
				pNode->pNext->pPrevious = pNode->pPrevious;
		}
		
		pList->itemCount--;
		if (pNode->type)
			free(pNode->type);
		free(pNode);
	}
}

// Delete the list and free the memory.
void listFree(DynamicListC *pList)
{
	while (listSelectFirst(*pList))
		listDeleteNode(*pList, listSelectCurrent(*pList));

	free(*pList);
	*pList = NULL;
}