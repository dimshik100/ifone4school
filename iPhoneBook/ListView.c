#include "StdAfx.h"
#include "ListView.h"
#include <windows.h>
#include <commctrl.h>

enum SORT { Sort_Ascending, Sort_Descending };
int CALLBACK sortListViewItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

// addListViewItem - adds columns to a list-view control.

// Returns TRUE if successful, or FALSE otherwise. 
// hWndListView - handle to the list-view control. 
// name - String containing contact name
// extra - String containing additional information about the item, can be NULL

BOOL addListViewItem(HWND hWndListView, TCHAR *name, TCHAR *extra)
{
	LVITEM lvItem = {0};

	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.state = 0;
	lvItem.stateMask = 0;

	lvItem.iItem = 0;	
	lvItem.iSubItem = 0;
	lvItem.pszText = name;
	lvItem.lParam = (LPARAM)1;
    if (ListView_InsertItem(hWndListView, &lvItem) == -1) 
		return FALSE; 

	if (extra)
	{		
		lvItem.iSubItem = 1;
		lvItem.pszText = extra;
		lvItem.lParam = (LPARAM)2;
		if (ListView_SetItem(hWndListView, &lvItem) == -1) 
			return FALSE; 
	}

	ListView_SortItems(hWndListView, sortListViewItems, Sort_Ascending);


    return TRUE; 
}

// initListViewColumns - adds columns to a list-view control.

// Returns TRUE if successful, or FALSE otherwise. 
// hWndListView - handle to the list-view control. 

BOOL initListViewColumns(HWND hWndListView) 
{ 
    TCHAR szText[256];     // temporary buffer 
    LVCOLUMN lvc; 

    // Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text, and subitem members
    // of the structure are valid. 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
	_tcscpy_s(szText, 256, TEXT("AAA"));
	  
    // Add the columns

	//Main column
	lvc.iSubItem = 0;
    lvc.pszText = szText;	
    lvc.cx = 400;     // width of column in pixels

	lvc.fmt = LVCFMT_LEFT;  // left-aligned column
	//lvc.fmt = LVCFMT_RIGHT; // right-aligned column
    if (ListView_InsertColumn(hWndListView, 0, &lvc) == -1) 
		return FALSE; 

	//Second column
	lvc.iSubItem = 0;
    lvc.pszText = szText;	
    lvc.cx = 50;     // width of column in pixels

	//lvc.fmt = LVCFMT_LEFT;  // left-aligned column
	lvc.fmt = LVCFMT_RIGHT; // right-aligned column
    if (ListView_InsertColumn(hWndListView, 1, &lvc) == -1) 
		return FALSE; 

    return TRUE; 
} 

// This function compares two items (it's a callback by the listview control)
// Depending on the selected order of sorting, it returns the correct information to the listview.
int CALLBACK sortListViewItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (lParamSort == Sort_Ascending)
	{
		if (lParam1 < lParam2)
			return -1;
		else if (lParam1 > lParam2)
			return 1;
	}
	else if (lParamSort == Sort_Descending)
	{
		if (lParam1 > lParam2)
			return -1;
		else if (lParam1 < lParam2)
			return 1;
	}
	
	return 0;
}

HWND createListView(HWND hWndParent, HINSTANCE hInstance)
{
	HWND hListViewWnd;
	hListViewWnd = CreateWindowEx(0, WC_LISTVIEW, NULL, 
		WS_CHILD | WS_VISIBLE | LVS_SHOWSELALWAYS | LVS_REPORT | LVS_SINGLESEL | /*LVS_OWNERDRAWFIXED |*/ LVS_NOCOLUMNHEADER | LVS_AUTOARRANGE,
		5, 5, 450, 150, hWndParent, NULL, hInstance, NULL);
	ListView_SetExtendedListViewStyle(hListViewWnd, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT);

	return hListViewWnd;
}