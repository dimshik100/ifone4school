#pragma once
#include "PhoneBook.h"

HWND createListView(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height);
BOOL initListViewColumns(HWND hWndListView);
BOOL addListViewItem(HWND hWndListView, Contact *contact);
LRESULT ListViewProc(HWND hWnd, WPARAM wParam, LPARAM lParam);
void fillListView(HWND hWndListView, DynamicListC pList, LPTSTR string);
LPARAM getListViewSelectedItemParam(HWND hWndListView);