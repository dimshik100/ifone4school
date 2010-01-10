#include "StdAfx.h"
#include "HoverButton.h"

#define MAX_BUTTONS 100
HoverButton *hoverButtons[MAX_BUTTONS];
int hoverButtonCounter = 0;

WNDPROC wndDefHoverBtnProc = NULL;
WNDPROC wndDefHoverProc = NULL;

LRESULT CALLBACK	HoverBtnProc(HWND, UINT, WPARAM, LPARAM);
void invalidateButtonRect(HoverButton *hoverButton);
void setHoverButtonImage(HoverButton *hoverButton, HDC hDC, int imageId);
BOOL setTrackMouse(HoverButton *hoverButton);

HoverButton *createHoverButton(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height, int controlId, int onImage, int offImage, TCHAR *caption)
{
	HoverButton *newHoverButton = NULL;

	if (hoverButtonCounter < MAX_BUTTONS)
	{
		newHoverButton = (HoverButton*)calloc(1, sizeof(HoverButton));
		newHoverButton->onImage = onImage;
		newHoverButton->offImage = offImage;
		newHoverButton->cId = controlId;
		newHoverButton->hInstance = hInstance;
		newHoverButton->buttonRect.left = x;
		newHoverButton->buttonRect.right = x + width;
		newHoverButton->buttonRect.top = y;
		newHoverButton->buttonRect.bottom = y + height;
		newHoverButton->hButton = CreateWindowEx(0, TEXT("button"), NULL, WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, x, y, width, height,
			hWndParent, (HMENU)controlId, hInstance, NULL);
		if (caption)
			newHoverButton->caption = _tcsdup(caption);
		newHoverButton->hFont = NULL;
		newHoverButton->color = GetSysColor(COLOR_BTNTEXT); // Get the default color for button text

		/// !!! Apply new window procedure to control !!! ///
		wndDefHoverProc = (WNDPROC)SetWindowLong(newHoverButton->hButton, GWL_WNDPROC, (LONG_PTR)HoverBtnProc);

		hoverButtons[hoverButtonCounter++] = newHoverButton;
	}

	return newHoverButton;
}

void setDefaultHoverButtonProc(WNDPROC wndProc)
{
	wndDefHoverBtnProc = wndProc;
}

HWND getHoverButtonHwnd(HoverButton *hoverButton)
{
	return hoverButton->hButton;
}

void deleteHoverButtons()
{
	int i;
	for (i = 0; i < hoverButtonCounter; i++)
	{
		if (hoverButtons[i]->hFont)
			DeleteObject(hoverButtons[i]->hFont);
		if (hoverButtons[i]->caption)
			free(hoverButtons[i]->caption);
		free(hoverButtons[i]);
	}
	hoverButtonCounter = 0;
}

void setHoverButtonStateImages(HoverButton *hoverButton, int onImage, int offImage)
{
	hoverButton->onImage = onImage;
	hoverButton->offImage = offImage;
	// Force redraw of the button
	invalidateButtonRect(hoverButton);
}

void setHoverButtonText(HoverButton *hoverButton, TCHAR *caption)
{
	if (hoverButton->caption)
		free(hoverButton->caption);
	if (caption)
		hoverButton->caption = _tcsdup(caption);
	else
		hoverButton->caption = NULL;
	// Force redraw of the button
	invalidateButtonRect(hoverButton);
}

void getHoverButtonText(HoverButton *hoverButton, TCHAR *destination, size_t length)
{
	if (hoverButton->caption)
		_tcscpy_s(destination, length, hoverButton->caption);
	else
		_tcscpy_s(destination, length, TEXT(""));
}

void setHoverButtonFont(HoverButton *hoverButton, TCHAR *fontName, int fontSize)
{
	HDC hDC;
	LOGFONT logFont = {0};

	hDC = GetDC(hoverButton->hButton);
	_tcscpy_s(logFont.lfFaceName, 32, fontName);
	logFont.lfHeight = -MulDiv(fontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	ReleaseDC(hoverButton->hButton, hDC);
	
	if (hoverButton->hFont)
		DeleteObject(hoverButton->hFont);
	hoverButton->hFont = CreateFontIndirect(&logFont);
	// Force redraw of the button
	invalidateButtonRect(hoverButton);
}

HFONT getHoverButtonFont(HoverButton *hoverButton)
{
	return hoverButton->hFont;
}

void setHoverButtonTextColor(HoverButton *hoverButton, COLORREF color)
{
	hoverButton->color = color;	
	// Force redraw of the button
	invalidateButtonRect(hoverButton);
}

void lockHoverButtonImage(HoverButton *hoverButton, int enable)
{
	hoverButton->isLocked = enable;
}

void setHoverButtonAsPushButton(HoverButton *hoverButton, int enable)
{
	hoverButton->isPushButton = enable;
}

BOOL setTrackMouse(HoverButton *hoverButton)
{
	TRACKMOUSEEVENT tme;
	
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = hoverButton->hButton;
	return TrackMouseEvent(&tme);
}

void setHoverButtonImage(HoverButton *hoverButton, HDC hDC, int imageId)
{
	HBITMAP hbmpOld, hbmpImage;
	HDC hDCMem;
	RECT rect;

	// Create a DC in memory, compatible with the button's original DC.
	hDCMem = CreateCompatibleDC(hDC);
	// Load the selected image from the resource file.
	hbmpImage = LoadBitmap(hoverButton->hInstance, MAKEINTRESOURCE(imageId));
	// Select the image into the DC. Keep a reference to the old bitmap.
	hbmpOld = (HBITMAP)SelectObject(hDCMem, hbmpImage);
	// Gets the dimensions of the button
	GetClientRect(hoverButton->hButton, &rect);
	// Copies the bitmap from the memory DC into the buttons DC 
	BitBlt(hDC, 0, 0, rect.right, rect.bottom, hDCMem, 0, 0, SRCCOPY);
	// Select the original memory DC's bitmap.
	SelectObject(hDCMem, hbmpOld);
	// Free resources.
	DeleteDC(hDCMem);
	DeleteObject(hbmpImage);
}

LRESULT CALLBACK	HoverBtnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HoverButton *hoverButton;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_LBUTTONDOWN:
		hoverButton = findButton(0, hWnd);
		if (!hoverButton->isLocked && hoverButton->isPushButton)
		{
			hoverButton->isHovering = TRUE;
			invalidateButtonRect(hoverButton);
			SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)MAKELONG(hoverButton->cId, HOVER_BUTTON_DOWN), (LPARAM)hWnd);
			return FALSE;
		}
		break;
	case WM_LBUTTONUP:
		hoverButton = findButton(0, hWnd);
		if (!hoverButton->isLocked && hoverButton->isPushButton)
		{
			hoverButton->isHovering = FALSE;
			invalidateButtonRect(hoverButton);
			SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)MAKELONG(hoverButton->cId, HOVER_BUTTON_UP), (LPARAM)hWnd);
			return FALSE;
		}
		break;
	case WM_MOUSEMOVE:
		{
			hoverButton = findButton(0, hWnd);
			// If the hover button is not locked AND
			// the hover button is not a push button AND is not hovering OR
			// the hover button is a push button AND the left mouse is down
			if (!hoverButton->isLocked && ((!hoverButton->isPushButton && !hoverButton->isHovering) || (hoverButton->isPushButton && (wParam & MK_LBUTTON))))
			{
				if (setTrackMouse(hoverButton))
				{
					hoverButton->isHovering = TRUE;

					// Force redraw of the button
					invalidateButtonRect(hoverButton);
				}
			}
			return FALSE;
		} 
		break;
	case WM_MOUSELEAVE:
		{
			hoverButton = findButton(0, hWnd);
			hoverButton->isHovering = FALSE;
			invalidateButtonRect(hoverButton);
			return FALSE;
		}
		break;

	case WM_PAINT:
		{
			HFONT hFontOld;
			HBITMAP hbmpOld, hbmpImage;
			HDC hDCMem;
			RECT rect;


			hdc = BeginPaint(hWnd, &ps);

			// Gets the dimensions of the button
			GetClientRect(hWnd, &rect);
			// Create a DC in memory, compatible with the button's original DC.
			hDCMem = CreateCompatibleDC(hdc);
			// Load the selected image from the resource file.
			hbmpImage = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
			// Select the image into the DC. Keep a reference to the old bitmap.
			hbmpOld = (HBITMAP)SelectObject(hDCMem, hbmpImage);

			hoverButton = findButton(0, hWnd);
			if (hoverButton->isHovering)
				setHoverButtonImage(hoverButton, hDCMem, hoverButton->onImage);
			else
				setHoverButtonImage(hoverButton, hDCMem, hoverButton->offImage);
			if (hoverButton->caption)
			{
				TEXTMETRIC tm;
				SetBkMode(hDCMem, TRANSPARENT);
				SetTextColor(hDCMem, hoverButton->color);
				SetTextAlign(hDCMem, TA_CENTER);
				hFontOld = (HFONT)SelectObject(hDCMem, hoverButton->hFont);
				GetTextMetrics(hDCMem, &tm);
				TextOut(hDCMem, (hoverButton->buttonRect.right - hoverButton->buttonRect.left)/2,
					(hoverButton->buttonRect.bottom - hoverButton->buttonRect.top - tm.tmHeight)/2,
					hoverButton->caption, (int)_tcslen(hoverButton->caption));
				SelectObject(hDCMem, hFontOld);
			}
			// Copies the bitmap from the memory DC into the buttons DC 
			BitBlt(hdc, 0, 0, rect.right, rect.bottom, hDCMem, 0, 0, SRCCOPY);
			// Select the original memory DC's bitmap.
			SelectObject(hDCMem, hbmpOld);
			// Free resources.
			DeleteDC(hDCMem);
			DeleteObject(hbmpImage);

			EndPaint(hWnd, &ps);

			return FALSE;
		}
		break;
	}
	return CallWindowProc(wndDefHoverProc, hWnd, message, wParam, lParam);
}

HoverButton *findButton(int cId, HWND hWnd)
{
	int i;

	if (cId)
	{
		for (i = 0; i < hoverButtonCounter; i++)
		{
			if (hoverButtons[i]->cId == cId)
				return hoverButtons[i];
		}
	}
	else if (hWnd)
	{
		for (i = 0; i < hoverButtonCounter; i++)
		{
			if (hoverButtons[i]->hButton == hWnd)
				return hoverButtons[i];
		}
	}

	return NULL;
}

void invalidateButtonRect(HoverButton *hoverButton)
{
	RECT rc;
	GetClientRect(hoverButton->hButton, &rc);
	InvalidateRect(hoverButton->hButton, &rc, FALSE);
}