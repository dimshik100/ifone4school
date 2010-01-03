#pragma once

typedef struct _HoverButton
{
	HWND hButton;
	HINSTANCE hInstance;
	HFONT hFont;
	COLORREF color;
	RECT buttonRect;
	TCHAR *caption;
	int isHovering;
	int isLocked;
	int isPushButton;
	int isPushed;
	int onImage;
	int offImage;
	int cId;

}HoverButton;

HoverButton *createHoverButton(HWND hWndParent, HINSTANCE hInstance, int x, int y,
							   int width, int height, int controlId, int onImage, int offImage, TCHAR *caption);
void setHoverButtonText(HoverButton *hoverButton, TCHAR *caption);
void setHoverButtonStateImages(HoverButton *hoverButton, int onImage, int offImage);
void setHoverButtonFont(HoverButton *hoverButton, TCHAR *fontName, int fontSize);
void setHoverButtonTextColor(HoverButton *hoverButton, COLORREF color);
void lockHoverButtonImage(HoverButton *hoverButton, int enable);
void setHoverButtonAsPushButton(HoverButton *hoverButton, int enable);
void setDefaultHoverButtonProc(WNDPROC wndProc);
HWND getHoverButtonHwnd(HoverButton *hoverButton);
void deleteHoverButtons();