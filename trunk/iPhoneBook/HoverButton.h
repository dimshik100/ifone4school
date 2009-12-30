#pragma once

typedef struct _HoverButton
{
	HWND hButton;
	HINSTANCE hInstance;
	RECT buttonRect;
	TCHAR *caption;
	int isHovering;
	int onImage;
	int offImage;
	int cId;

}HoverButton;

HoverButton *createHoverButton(HWND hWndParent, HINSTANCE hInstance, int x, int y,
							   int width, int height, int controlId, int onImage, int offImage, TCHAR *caption);
void setHoverButtonImage(HoverButton *hoverButton, HDC hDC, int imageId);
void setHoverButtonText(HoverButton *hoverButton, TCHAR *caption);
void setHoverButtonFont(HoverButton *hoverButton, TCHAR *fontName, int fontSize);
void setDefaultHoverButtonProc(WNDPROC wndProc);