#pragma once

HWND createListView(HWND hWndParent, HINSTANCE hInstance);
BOOL initListViewColumns(HWND hWndListView);
BOOL addListViewItem(HWND hWndListView, TCHAR *name, TCHAR *extra);