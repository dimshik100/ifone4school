#include "StdAfx.h"
#include "ListView.h"
#include "resource.h"
#include <windows.h>
#include <commctrl.h>

WNDPROC wndDefListViewProc = NULL;
HINSTANCE hInst = NULL;
HWND hwndListView = NULL;

enum SORT { Sort_Ascending, Sort_Descending };
extern void setImageToDC(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hDC, int imageId);
int CALLBACK sortListViewItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int compareNames(Contact *target, LPTSTR name);

void fillListView(DynamicListC pList, LPTSTR string)
{
	Contact *contact;

	if (!pList)
		return;

	ListView_DeleteAllItems(hwndListView);
	if (string && _tcslen(string) > 0)
	{
		_tcslwr_s(string, _tcslen(string) + 1);
		for (listSelectFirst(pList); listSelectCurrent(pList); listSelectNext(pList, NULL))
		{
			listGetValue(pList, NULL, &contact);
			if (compareNames(contact, string))
				addListViewItem(hwndListView, contact);
		}
	}
	else
	{
		for (listSelectFirst(pList); listSelectCurrent(pList); listSelectNext(pList, NULL))
		{
			listGetValue(pList, NULL, &contact);
			addListViewItem(hwndListView, contact);
		}
	}
}

// addListViewItem - adds columns to a list-view control.

// Returns TRUE if successful, or FALSE otherwise. 
// hWndListView - handle to the list-view control. 
// name - String containing contact name
// extra - String containing additional information about the item, can be NULL

BOOL addListViewItem(HWND hWndListView, Contact *contact)
{
	LVITEM lvItem = {0};
	TCHAR string[256];

	lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvItem.state = 0;
	lvItem.stateMask = 0;

	_stprintf_s(string, 256, TEXT("%s %s"), contact->lastName, contact->firstName);

	lvItem.iItem = 0;	
	lvItem.iSubItem = 0;
	lvItem.pszText = string;
	lvItem.lParam = (LPARAM)contact;
    if (ListView_InsertItem(hWndListView, &lvItem) == -1) 
		return FALSE;

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
    lvc.cx = 319;     // width of column in pixels

	lvc.fmt = LVCFMT_LEFT;  // left-aligned column
	//lvc.fmt = LVCFMT_RIGHT; // right-aligned column
    if (ListView_InsertColumn(hWndListView, 0, &lvc) == -1) 
		return FALSE; 

	////Second column
	//lvc.iSubItem = 0;
 //   lvc.pszText = szText;	
 //   lvc.cx = 50;     // width of column in pixels

	////lvc.fmt = LVCFMT_LEFT;  // left-aligned column
	//lvc.fmt = LVCFMT_RIGHT; // right-aligned column
 //   if (ListView_InsertColumn(hWndListView, 1, &lvc) == -1) 
	//	return FALSE; 

    return TRUE; 
} 

// This function compares two items (it's a callback by the listview control)
// Depending on the selected order of sorting, it returns the correct information to the listview.
int CALLBACK sortListViewItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	Contact *contact1, *contact2;
	int sortType = 0, ret;

	if (!lParam1)
		return -1;
	else if (!lParam2)
		return 1;

	contact1 = (Contact*)lParam1;
	contact2 = (Contact*)lParam2;
	if (lParamSort == Sort_Ascending)
		sortType = 1;
	else if (lParamSort == Sort_Descending)
		sortType = -1;

	ret = sortType*(_tcsicmp(contact1->lastName, contact2->lastName));
	if (!ret)
		ret = sortType*(_tcsicmp(contact1->firstName, contact2->firstName));

	return ret;
}

HWND createListView(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height)
{
	HIMAGELIST imageList = ImageList_Create(1, 43, ILC_COLORDDB, 0, 0);
	hwndListView = CreateWindowEx(0, WC_LISTVIEW, NULL, 
		WS_CHILD | WS_VISIBLE | LVS_SHOWSELALWAYS | LVS_REPORT | LVS_SINGLESEL | /*LVS_OWNERDRAWFIXED |*/ LVS_NOCOLUMNHEADER | LVS_AUTOARRANGE,
		x, y, width, height, hWndParent, NULL, hInstance, NULL);
	ShowScrollBar(hwndListView, SB_VERT, FALSE);
	ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT);
	ListView_SetImageList(hwndListView, imageList, LVSIL_SMALL);
	hInst = hInstance;

	return hwndListView;
}

int compareNames(Contact *target, LPTSTR name)
{
	TCHAR origName[200], lastName[100], firstName[100], fullName1[200], fullName2[200], *tempStr;

	_tcscpy_s(origName, 200, name);
	_tcslwr_s(origName, _tcslen(origName) + 1);

	_tcscpy_s(lastName, 100, target->lastName);
	_tcslwr_s(lastName, _tcslen(lastName) + 1);

	_tcscpy_s(firstName, 100, target->firstName);
	_tcslwr_s(firstName, _tcslen(firstName) + 1);

	_stprintf_s(fullName1, 200, TEXT("%s %s"), lastName, firstName);
	_tcslwr_s(fullName1, _tcslen(fullName1) + 1);

	_stprintf_s(fullName2, 200, TEXT("%s %s"), firstName, lastName);
	_tcslwr_s(fullName2, _tcslen(fullName2) + 1);

	tempStr = _tcsstr(lastName, origName);
	if (tempStr && tempStr == lastName)
		return TRUE;
	tempStr = _tcsstr(firstName, origName);
	if (tempStr && tempStr == lastName)
		return TRUE;
	tempStr = _tcsstr(fullName1, origName);
	if (tempStr && tempStr == fullName1)
		return TRUE;
	tempStr = _tcsstr(fullName2, origName);
	if (tempStr && tempStr == fullName2)
		return TRUE;
	return FALSE;
}

LRESULT	ListViewProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd), UNREFERENCED_PARAMETER(wParam);
	if (lParam && ((LPNMHDR)lParam)->hwndFrom == hwndListView && ((LPNMHDR)lParam)->code == NM_CUSTOMDRAW)
	{
		LPNMLVCUSTOMDRAW lpNMCustomDraw = (LPNMLVCUSTOMDRAW) lParam;
		switch (lpNMCustomDraw->nmcd.dwDrawStage)
		{
		//case CDDS_POSTERASE:
		//	break;
		//case CDDS_POSTPAINT:
		//	break;
		//case CDDS_PREERASE:
		//	break;
		case CDDS_PREPAINT:
			// This event happens when the control has to be redrawn, 
			// to redraw all the subitems, we need to return the following:
			return (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW);
			break;
		//case CDDS_ITEM:
		//	break;
		//case CDDS_ITEMPOSTERASE:
		//	break;
		case CDDS_ITEMPOSTPAINT:
			// This event happens after Windows is done repainting the control.
			// If this is an item, go into the "IF"
			if (lpNMCustomDraw->dwItemType == LVCDI_ITEM)
			{
				RECT rcOffset = {0};
				TCHAR str[1000];
				TEXTMETRIC tm;
				int textTop;

				// Get current item's rect.
				ListView_GetItemRect(lpNMCustomDraw->nmcd.hdr.hwndFrom, lpNMCustomDraw->nmcd.dwItemSpec, &lpNMCustomDraw->nmcd.rc, LVIR_BOUNDS);
				lpNMCustomDraw->nmcd.uItemState = ListView_GetItemState(lpNMCustomDraw->nmcd.hdr.hwndFrom, lpNMCustomDraw->nmcd.dwItemSpec, LVIS_SELECTED);
				lpNMCustomDraw->nmcd.rc.right++;
				// Get current item's text
				ListView_GetItemText(lpNMCustomDraw->nmcd.hdr.hwndFrom, lpNMCustomDraw->nmcd.dwItemSpec, 0, str, 1000);
				// If item is being hovered on, draw a differet colored rect around it.
				// Else draw it with blue-ish background
				if (lpNMCustomDraw->nmcd.uItemState & LVIS_SELECTED)
					setImageToDC(hInst, &lpNMCustomDraw->nmcd.rc, &rcOffset, lpNMCustomDraw->nmcd.hdc, IDB_CONTACT_WND_NAME_BG_ON);
				else
					setImageToDC(hInst, &lpNMCustomDraw->nmcd.rc, &rcOffset, lpNMCustomDraw->nmcd.hdc, IDB_CONTACT_WND_NAME_BG_OFF);
				// Print text to item's DC.
				GetTextMetrics(lpNMCustomDraw->nmcd.hdc, &tm);
				textTop = lpNMCustomDraw->nmcd.rc.top + (lpNMCustomDraw->nmcd.rc.bottom - lpNMCustomDraw->nmcd.rc.top - tm.tmHeight) / 2;

				TextOut(lpNMCustomDraw->nmcd.hdc, lpNMCustomDraw->nmcd.rc.left + 5, textTop, str, _tcslen(str));
			}
			break;
		//case CDDS_ITEMPREERASE:
		//	break;
		case CDDS_ITEMPOSTERASE:
			if (lpNMCustomDraw->dwItemType == LVCDI_ITEM)
			{
				RECT rc, rcOffset = {0};
				ListView_GetItemRect(lpNMCustomDraw->nmcd.hdr.hwndFrom, lpNMCustomDraw->nmcd.dwItemSpec, &rc, LVIR_BOUNDS);
				setImageToDC(hInst, &rc, &rcOffset, lpNMCustomDraw->nmcd.hdc, IDB_CONTACT_WND_NAME_BG_ON);
			}
			return CDRF_DODEFAULT;
			break;
		case CDDS_ITEMPREPAINT:
			// Make sure CDDS_ITEMPOSTPAINT occurs.
			lpNMCustomDraw->rcText.left = 0; 
			lpNMCustomDraw->rcText.top = 0; 
			lpNMCustomDraw->rcText.right = 320; 
			lpNMCustomDraw->rcText.bottom = 44; 
			return (CDRF_NOTIFYPOSTPAINT);
			break;
		//case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		//	//return (CDRF_NOTIFYPOSTPAINT);
		//	break;
		//case CDDS_SUBITEM | CDDS_ITEMPOSTPAINT:
		//	break;
		//default:
		//	return CDRF_DODEFAULT;
		//	break;
		}
	}
		
	return FALSE;
}