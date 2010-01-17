#pragma once

HWND createScrollContainer(HWND hWndParent, HINSTANCE hInstance, DWORD dwStyle, int x, int y, int width, int height, int itemWidth, int itemHeight, int virtWidth, int virtHeight, int controlId, int bgImage);
void setScrollContainerSize(HWND hWnd, SIZE *size, SIZE *virtSize);
void setScrollContainerImageStretch(HWND hWnd, int enable);

void setScrollContainerText(HWND hWnd, TCHAR *caption);
size_t getScrollContainerText(HWND hWnd, TCHAR *destination, size_t length);
void setScrollContainerImage(HWND hWnd, int imgImage);
void setScrollContainerImageStretch(HWND hWnd, int enable);
void setScrollContainerFont(HWND hWnd, TCHAR *fontName, int fontSize);
HFONT getScrollContainerFont(HWND hWnd);
void setScrollContainerTextColor(HWND hWnd, COLORREF color);