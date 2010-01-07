#pragma once

HWND createListView(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height);
BOOL initListViewColumns(HWND hWndListView);
BOOL addListViewItem(HWND hWndListView, TCHAR *name);