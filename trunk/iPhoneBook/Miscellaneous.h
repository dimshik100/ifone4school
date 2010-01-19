#pragma once

void setImageToDc(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId);
void setImageToDcStretched(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId);
void setBitmapToDc(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage);
void setBitmapToDcStretched(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage);
void showChildWindows(HWND hWnd, int nCmdShow);
void shiftChildWindows(HWND hWnd, int xOffset, int yOffset, BOOL bErase);
void invalidateChildWindows(HWND hWnd, BOOL bErase);
void getChildInParentOffset(HWND hWnd, POINT *lppt);
int isOsVista();
void rectToSize(RECT *rc, SIZE *size);
void makeWindowTransparentByMask(HWND hWnd, int mask);
void getModulePath(LPWCH lpFilename, DWORD nSize);
int loadCustomFont(LPTSTR fontFileName);
BOOL unloadCustomFont(LPTSTR fontFileName);

#define ROUND(floatVal)(((floatVal)-(int)(floatVal)) >= 0.5 ? ((int)(floatVal) + 1) : ((int)(floatVal)))