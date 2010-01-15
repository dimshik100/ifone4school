#pragma once

HWND createScrollContainer(HWND hWndParent, HINSTANCE hInstance, DWORD dwStyle, int x, int y, int width, int height, int virtWidth, int virtHeight, int controlId, int bgImage);
void setScrollContainerSize(HWND hWnd, SIZE *size, SIZE *virtSize);
void setScrollContainerImageStretch(HWND hWnd, int enable);
