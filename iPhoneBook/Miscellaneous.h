#pragma once
#include "DynamicListC.h"

void setImageToDc(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId);
void setImageToDcStretched(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId);
void setBitmapToDc(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage);
void setBitmapToDcStretched(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage);
DynamicListC getContactListLocal();
DynamicListC getContactListFromFile();
DynamicListC getContactListInitiated();
void showChildWindows(HWND hwnd, int nCmdShow);