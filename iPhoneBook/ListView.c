#include "StdAfx.h"
#include "ListView.h"
#include "Miscellaneous.h"
#include "resource.h"
#include <windows.h>
#include <commctrl.h>

WNDPROC wndDefListViewProc = NULL;
HINSTANCE hInst = NULL;
HWND hwndListView = NULL;
HFONT hFontBold = NULL, hFontNormal = NULL;

enum SORT { Sort_Ascending, Sort_Descending };
int CALLBACK sortListViewItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int compareNames(Contact *target, LPTSTR name);
void adjustColumnWidth(HWND hWndListView);

void fillListView(HWND hWndListView, DynamicListC pList, LPTSTR string)
{
	Contact *contact;
	int i, listCount, chr = 0;
	TCHAR tempStr[1000];
	LVITEM lvItem = {0};

	if (!pList)
		return;

	ListView_DeleteAllItems(hWndListView);
	if (string && _tcslen(string) > 0)
	{
		_tcslwr_s(string, _tcslen(string) + 1);
		for (listSelectFirst(pList); listSelectCurrent(pList); listSelectNext(pList, NULL))
		{
			listGetValue(pList, NULL, &contact);
			if (compareNames(contact, string))
				addListViewItem(hWndListView, contact);
		}
	}
	else
	{
		for (listSelectFirst(pList); listSelectCurrent(pList); listSelectNext(pList, NULL))
		{
			listGetValue(pList, NULL, &contact);
			addListViewItem(hWndListView, contact);
		}
	}
	ListView_SortItems(hWndListView, sortListViewItems, Sort_Ascending);
	listCount = ListView_GetItemCount(hWndListView);
	for (i = 0; i < listCount; i++)
	{
		lvItem.iItem = i;
		lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
		lvItem.pszText = tempStr;
		lvItem.cchTextMax = 1000;
		ListView_GetItem(hWndListView, &lvItem);
		if (lvItem.lParam)
		{
			if (chr != toupper(((Contact*)lvItem.lParam)->lastName[0]))
			{
				chr = toupper(((Contact*)lvItem.lParam)->lastName[0]);
				_stprintf_s(lvItem.pszText, lvItem.cchTextMax, TEXT("%c"), chr);
				lvItem.lParam = (LPARAM)NULL;
				if (ListView_InsertItem(hWndListView, &lvItem) > -1)
					listCount++, i++;
			}
		}
	}
	adjustColumnWidth(hWndListView);
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

	adjustColumnWidth(hWndListView);

    return TRUE; 
}

void adjustColumnWidth(HWND hWndListView)
{
	int cx;
	if (ListView_GetItemCount(hWndListView) > 6)
	{
		cx = 319 - GetSystemMetrics(SM_CXVSCROLL);
		if (ListView_GetColumnWidth(hWndListView, 0) == 319)
			ListView_SetColumnWidth(hWndListView, 0, cx);
	}
	else if (ListView_GetColumnWidth(hWndListView, 0) < 319)
		ListView_SetColumnWidth(hWndListView, 0, 319);
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
	HDC hdc;
	HIMAGELIST imageList = ImageList_Create(1, 43, ILC_COLORDDB, 0, 0);
	hwndListView = CreateWindowEx(0, WC_LISTVIEW, NULL, 
		WS_CHILD | WS_VISIBLE | LVS_SHOWSELALWAYS | LVS_REPORT | LVS_SINGLESEL | /*LVS_OWNERDRAWFIXED |*/ LVS_NOCOLUMNHEADER | LVS_AUTOARRANGE,
		x, y, width, height, hWndParent, NULL, hInstance, NULL);
	ShowScrollBar(hwndListView, SB_VERT, FALSE);
	ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT);
	ListView_SetImageList(hwndListView, imageList, LVSIL_SMALL);
	hInst = hInstance;

	hdc = GetDC(hwndListView);
	hFontBold = CreateFont(-MulDiv(14, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, TEXT("Arial"));
	hFontNormal = CreateFont(-MulDiv(14, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, FW_DONTCARE, 0, 0, 0, DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, TEXT("Arial"));
	ReleaseDC(hwndListView, hdc);

	return hwndListView;
}

int compareNames(Contact *target, LPTSTR name)
{
	TCHAR origName[200], lastName[100], firstName[100], *token, *nextToken, delims[] = TEXT(" "), *tempStr;
	int ret;

	_tcscpy_s(origName, 200, name);
	_tcslwr_s(origName, _tcslen(origName) + 1);

	_tcscpy_s(lastName, 100, target->lastName);
	_tcslwr_s(lastName, _tcslen(lastName) + 1);

	_tcscpy_s(firstName, 100, target->firstName);
	_tcslwr_s(firstName, _tcslen(firstName) + 1);

	token = _tcstok_s(origName, delims, &nextToken);
	while (token)
	{
		ret = FALSE;
		if (_tcslen(lastName))
		{
			tempStr = _tcsstr(lastName, token);
			if (tempStr && tempStr == lastName)
			{
				_tcscpy_s(lastName, 100, TEXT(""));
				ret = TRUE;
			}
		}
		if (_tcslen(firstName))
		{
			tempStr = _tcsstr(firstName, token);
			if (tempStr && tempStr == firstName)
			{
				_tcscpy_s(firstName, 100, TEXT(""));
				ret = TRUE;
			}
		}
		if (!ret)
			break;
		token = _tcstok_s(NULL, delims, &nextToken);
	}
	return ret;
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
				LVITEM lvItem = {0};
				int textTop;
				HFONT hFont;
				Contact *contact = NULL;

				// Get current item's rect.
				lvItem.iItem = lpNMCustomDraw->nmcd.dwItemSpec;
				lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
				lvItem.stateMask = LVIS_SELECTED;
				lvItem.pszText = str;
				lvItem.cchTextMax = 1000;
				ListView_GetItem(lpNMCustomDraw->nmcd.hdr.hwndFrom, &lvItem);
				ListView_GetItemRect(lpNMCustomDraw->nmcd.hdr.hwndFrom, lpNMCustomDraw->nmcd.dwItemSpec, &lpNMCustomDraw->nmcd.rc, LVIR_BOUNDS);
				lpNMCustomDraw->nmcd.rc.right++;
				// If item is a "letter seperator"
				// Else If item is being hovered on, draw a differet colored rect around it.
				// Else draw it with blue-ish background
				contact = (Contact*)lvItem.lParam;
				if (!contact)
					setImageToDcStretched(hInst, &lpNMCustomDraw->nmcd.rc, &rcOffset, lpNMCustomDraw->nmcd.hdc, IDB_CONTACT_WND_LET_SEP);
				else if (lvItem.state & LVIS_SELECTED)
					setImageToDcStretched(hInst, &lpNMCustomDraw->nmcd.rc, &rcOffset, lpNMCustomDraw->nmcd.hdc, IDB_CONTACT_WND_NAME_BG_ON);
				else
					setImageToDcStretched(hInst, &lpNMCustomDraw->nmcd.rc, &rcOffset, lpNMCustomDraw->nmcd.hdc, IDB_CONTACT_WND_NAME_BG_OFF);
				// Print text to item's DC.

				rcOffset.left = 10;
				SetBkMode(lpNMCustomDraw->nmcd.hdc, TRANSPARENT);
				if (contact)
				{
					hFont = (HFONT)SelectObject(lpNMCustomDraw->nmcd.hdc, hFontNormal);
					GetTextMetrics(lpNMCustomDraw->nmcd.hdc, &tm);

					DrawTextEx(lpNMCustomDraw->nmcd.hdc, contact->firstName, _tcslen(contact->firstName), &rcOffset, DT_CALCRECT, NULL);
					textTop = lpNMCustomDraw->nmcd.rc.top + (lpNMCustomDraw->nmcd.rc.bottom - lpNMCustomDraw->nmcd.rc.top - tm.tmHeight) / 2;
					TextOut(lpNMCustomDraw->nmcd.hdc, lpNMCustomDraw->nmcd.rc.left + 10, textTop, contact->firstName, _tcslen(contact->firstName));

					SelectObject(lpNMCustomDraw->nmcd.hdc, hFontBold);
					GetTextMetrics(lpNMCustomDraw->nmcd.hdc, &tm);
					textTop = lpNMCustomDraw->nmcd.rc.top + (lpNMCustomDraw->nmcd.rc.bottom - lpNMCustomDraw->nmcd.rc.top - tm.tmHeight) / 2;

					rcOffset.left = rcOffset.right;
					DrawTextEx(lpNMCustomDraw->nmcd.hdc, TEXT(" "), _tcslen(TEXT(" ")), &rcOffset, DT_CALCRECT, NULL);					
					TextOut(lpNMCustomDraw->nmcd.hdc, lpNMCustomDraw->nmcd.rc.left + rcOffset.left, textTop, TEXT(" "), _tcslen(TEXT(" ")));
					rcOffset.left = rcOffset.right;
					DrawTextEx(lpNMCustomDraw->nmcd.hdc, lvItem.pszText, _tcslen(lvItem.pszText), &rcOffset, DT_CALCRECT, NULL);					
					TextOut(lpNMCustomDraw->nmcd.hdc, lpNMCustomDraw->nmcd.rc.left + rcOffset.left, textTop, contact->lastName, _tcslen(contact->lastName));
				}
				else
				{
					COLORREF origColor = GetTextColor(lpNMCustomDraw->nmcd.hdc);
					SetTextColor(lpNMCustomDraw->nmcd.hdc, RGB(255, 255, 255));
					hFont = (HFONT)SelectObject(lpNMCustomDraw->nmcd.hdc, hFontBold);
					GetTextMetrics(lpNMCustomDraw->nmcd.hdc, &tm);
					DrawTextEx(lpNMCustomDraw->nmcd.hdc, lvItem.pszText, _tcslen(lvItem.pszText), &rcOffset, DT_CALCRECT, NULL);
					textTop = lpNMCustomDraw->nmcd.rc.top + (lpNMCustomDraw->nmcd.rc.bottom - lpNMCustomDraw->nmcd.rc.top - tm.tmHeight) / 2;
					TextOut(lpNMCustomDraw->nmcd.hdc, lpNMCustomDraw->nmcd.rc.left + 10, textTop, lvItem.pszText, _tcslen(lvItem.pszText));
					SetTextColor(lpNMCustomDraw->nmcd.hdc, origColor);
				}

				SelectObject(lpNMCustomDraw->nmcd.hdc, hFont);

			}
			break;
		//case CDDS_ITEMPREERASE:
		//	break;
		//case CDDS_ITEMPOSTERASE:
		//	break;
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