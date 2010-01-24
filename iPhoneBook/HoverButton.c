#include "StdAfx.h"
#include "HoverButton.h"
#include "Miscellaneous.h"
#include "DynamicListC.h"

WNDPROC wndDefHoverProc = NULL;

LRESULT CALLBACK	HoverBtnProc(HWND, UINT, WPARAM, LPARAM);
void invalidateButtonRect(HoverButton *hoverButton);
void setHoverButtonImage(HoverButton *hoverButton, HDC hdc, int imageId);
BOOL setTrackMouse(HoverButton *hoverButton);
void paintButton(HWND hWnd, HDC hdc, RECT *rcPaint);

DynamicListC hoverButtonList = NULL;

HoverButton *createHoverButton(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height,
							   int controlId, int onImage, int offImage, TCHAR *caption)
{
	HoverButton *newHoverButton = (HoverButton*)calloc(1, sizeof(HoverButton));

	if (!hoverButtonList)
		listInit(&hoverButtonList);

	listInsertAfterEnd(hoverButtonList, &newHoverButton);

	newHoverButton->onImage = onImage;
	newHoverButton->offImage = offImage;
	newHoverButton->activeImage = offImage;
	newHoverButton->imgStretch = FALSE;
	newHoverButton->cId = controlId;
	newHoverButton->hInstance = hInstance;
	newHoverButton->buttonRect.left = x;
	newHoverButton->buttonRect.right = x + width;
	newHoverButton->buttonRect.top = y;
	newHoverButton->buttonRect.bottom = y + height;
	newHoverButton->hButton = CreateWindowEx(0, TEXT("button"), NULL, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | BS_OWNERDRAW, x, y, width, height,
		hWndParent, (HMENU)controlId, hInstance, NULL);
	if (caption)
		newHoverButton->caption = _tcsdup(caption);
	newHoverButton->hFont = NULL;
	newHoverButton->color = GetSysColor(COLOR_BTNTEXT); // Get the default color for button text

	/// !!! Apply new window procedure to control !!! ///
	wndDefHoverProc = (WNDPROC)SetWindowLong(newHoverButton->hButton, GWL_WNDPROC, (LONG_PTR)HoverBtnProc);

	return newHoverButton;
}


// Gets the HWND of the control
HWND getHoverButtonHwnd(HoverButton *hoverButton)
{
	return hoverButton->hButton;
}

// Frees the list of EditButton objects
void deleteHoverButtons()
{
	HoverButton *hoverButton;
	for (listSelectFirst(hoverButtonList); listSelectCurrent(hoverButtonList); listSelectNext(hoverButtonList, NULL))
	{
		listGetValue(hoverButtonList, NULL, &hoverButton);
		if (hoverButton->hFont)
			DeleteObject(hoverButton->hFont);
		if (hoverButton->caption)
			free(hoverButton->caption);
	}
	listFree(&hoverButtonList);
}

// Sets the active and inactive image
void setHoverButtonStateImages(HoverButton *hoverButton, int onImage, int offImage)
{
	hoverButton->onImage = onImage;
	hoverButton->offImage = offImage;
	hoverButton->activeImage = offImage;
	// Force redraw of the button
	invalidateButtonRect(hoverButton);
}

// Enables or disables stretching the image into the control's Rect
void setHoverButtonImageStretch(HoverButton *hoverButton, int enable)
{
	hoverButton->imgStretch = enable;
}

// Sets the text into the control
void setHoverButtonText(HoverButton *hoverButton, TCHAR *caption)
{
	if (hoverButton->caption)
		free(hoverButton->caption);
	if (caption)
	{
		//hoverButton->caption = _tcsdup(caption);
		hoverButton->caption = (TCHAR*)malloc(sizeof(TCHAR)*(_tcslen(caption)+1));
		_tcscpy_s(hoverButton->caption, _tcslen(caption)+1, caption);
	}
	else
		hoverButton->caption = NULL;
	// Force redraw of the button
	invalidateButtonRect(hoverButton);
}

// Gets the text string from the control
void getHoverButtonText(HoverButton *hoverButton, TCHAR *destination, size_t length)
{
	if (hoverButton->caption)
		_tcscpy_s(destination, length, hoverButton->caption);
	else
		_tcscpy_s(destination, length, TEXT(""));
}

// Sets a new font to the control
void setHoverButtonFont(HoverButton *hoverButton, TCHAR *fontName, int fontSize, int isBold)
{
	int boldVal = (isBold) ? FW_BOLD : FW_DONTCARE;
	if (hoverButton->hFont)
		DeleteObject(hoverButton->hFont);
	hoverButton->hFont = CreateFont(getFontHeight(hoverButton->hButton, fontSize), 0, 0, 0, boldVal, 0, 0, 0, DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, fontName);
	// Force redraw of the button
	invalidateButtonRect(hoverButton);
}

// Gets the font in the current control
HFONT getHoverButtonFont(HoverButton *hoverButton)
{
	return hoverButton->hFont;
}

// Sets the text color of the control
void setHoverButtonTextColor(HoverButton *hoverButton, COLORREF color)
{
	hoverButton->color = color;	
	// Force redraw of the button
	invalidateButtonRect(hoverButton);
}

// Locks the control to only display the "off image"
void lockHoverButtonImage(HoverButton *hoverButton, int enable)
{
	hoverButton->isLocked = enable;
	if (enable)
	{
		hoverButton->isPushed = FALSE;
		hoverButton->isHovering = FALSE;
		hoverButton->activeImage = hoverButton->offImage;
	}
	invalidateButtonRect(hoverButton);
}

// Makes the hover button function similar to a standard Windows PushButton
void setHoverButtonAsPushButton(HoverButton *hoverButton, int enable)
{
	hoverButton->isPushButton = enable;
}

// Enables the firing of a mouse tracking event for when the mouse leaves the control area
BOOL setTrackMouse(HoverButton *hoverButton)
{
	TRACKMOUSEEVENT tme;
	
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = hoverButton->hButton;
	return TrackMouseEvent(&tme);
}

// Selects the proper way to draw the image on the HoverButton
void setHoverButtonImage(HoverButton *hoverButton, HDC hdc, int imageId)
{
	RECT rect, rcOffset = {0};
	// Gets the dimensions of the button
	GetClientRect(hoverButton->hButton, &rect);

	if (hoverButton->imgStretch)
		setImageToDcStretched(hoverButton->hInstance, &rect, &rcOffset, hdc, imageId);
	else
		setImageToDc(hoverButton->hInstance, &rect, &rcOffset, hdc, imageId);
}

// The sub-classing procedure to handle messages of the control
LRESULT CALLBACK	HoverBtnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HoverButton *hoverButton;

	switch (message)
	{
	// If we get a WM_COMMAND message, pass it on to the parent control to handle.
	// This is very useful when we have a nested controls in our HoverButton
	case WM_COMMAND:
		SendMessage(GetParent(hWnd), WM_COMMAND, wParam, lParam);
		break;
	// Button notifications sets the appropriate state to the button and notifies the parent
	case WM_RBUTTONDOWN:
		hoverButton = findHoverButton(0, hWnd);
		hoverButton->isPushed = TRUE;
		if (!hoverButton->isLocked)
			SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)MAKELONG(hoverButton->cId, HOVER_BUTTON_RMOUSE_DOWN), (LPARAM)hWnd);
		return FALSE;
		break;
	case WM_RBUTTONUP:
		hoverButton = findHoverButton(0, hWnd);
		hoverButton->isPushed = FALSE;
		if (!hoverButton->isLocked)
			SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)MAKELONG(hoverButton->cId, HOVER_BUTTON_RMOUSE_UP), (LPARAM)hWnd);
		return FALSE;
		break;
	case WM_LBUTTONDOWN:
		SetFocus(hWnd);
		hoverButton = findHoverButton(0, hWnd);
		hoverButton->isPushed = TRUE;
		if (!hoverButton->isLocked)
		{
			hoverButton->activeImage = hoverButton->onImage;
			hoverButton->isHovering = TRUE;
			invalidateButtonRect(hoverButton);
			SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)MAKELONG(hoverButton->cId, HOVER_BUTTON_LMOUSE_DOWN), (LPARAM)hWnd);
		}
		return FALSE;
		break;
	case WM_LBUTTONUP:
		hoverButton = findHoverButton(0, hWnd);
		if (!hoverButton->isLocked)
		{
			if (hoverButton->isPushButton)
			{
				hoverButton->activeImage = hoverButton->offImage;
				hoverButton->isHovering = FALSE;
				invalidateButtonRect(hoverButton);
			}
			if (hoverButton->isPushed)
				SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)MAKELONG(hoverButton->cId, HOVER_BUTTON_LMOUSE_UP), (LPARAM)hWnd);
		}
		hoverButton->isPushed = FALSE;
		return FALSE;
		break;
	// When mouse moves over the control, enable the WM_MOUSELEAVE event and set image to active
	case WM_MOUSEMOVE:
		{
			hoverButton = findHoverButton(0, hWnd);
			// If the hover button is not locked AND
			// the hover button is not a push button AND is not hovering OR
			// the hover button is a push button AND the left mouse is down
			if (!hoverButton->isLocked && ((!hoverButton->isPushButton && !hoverButton->isHovering) || (hoverButton->isPushButton && (wParam & MK_LBUTTON))))
			{
				if (setTrackMouse(hoverButton))
				{
					hoverButton->activeImage = hoverButton->onImage;
					hoverButton->isHovering = TRUE;

					// Force redraw of the button
					invalidateButtonRect(hoverButton);
				}
			}
			return FALSE;
		} 
		break;
	// When mouse leaves the control set image to inactive and notify parent
	case WM_MOUSELEAVE:
		{
			hoverButton = findHoverButton(0, hWnd);

			if (!hoverButton->isLocked)
			{
				hoverButton->activeImage = hoverButton->offImage;
				hoverButton->isHovering = FALSE;
				invalidateButtonRect(hoverButton);
				if (hoverButton->isPushed)
					SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)MAKELONG(hoverButton->cId, HOVER_BUTTON_MOUSE_DOWN_LEAVE), (LPARAM)hWnd);
			}
			return FALSE;
		}
		break;
	// On resize, adjust the Rect of the HoverButton control to the new dimensions.
	case WM_SIZE:
		{
			hoverButton = findHoverButton(0, hWnd);
			if (hoverButton)
			{
				hoverButton->buttonRect.right = hoverButton->buttonRect.left + (short)LOWORD(lParam);
				hoverButton->buttonRect.bottom = hoverButton->buttonRect.top + (short)HIWORD(lParam);
			}
		}
		break;
	// On move adjust the Rect to the new position
	case WM_MOVE:
		{
			hoverButton = findHoverButton(0, hWnd);
			if (hoverButton)
			{
				SIZE size;
				rectToSize(&hoverButton->buttonRect, &size);
				hoverButton->buttonRect.left = (short)LOWORD(lParam);
				hoverButton->buttonRect.top = (short)HIWORD(lParam);
				hoverButton->buttonRect.right = hoverButton->buttonRect.left + size.cx;
				hoverButton->buttonRect.bottom = hoverButton->buttonRect.top + size.cy;
			}
		}
		break;
	// Disable erasing the control's background, we always repaint it fully anyway
	case WM_ERASEBKGND:
		return TRUE;
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hWnd, &ps);
			paintButton(hWnd, hdc, &ps.rcPaint);
			EndPaint(hWnd, &ps);
			return FALSE;
		}
		break;
	// During animations, we must handle these messages for the control to render properly.
	case WM_PRINT:
	case WM_PRINTCLIENT:
		if (lParam & PRF_CLIENT)
		{
			RECT rcPaint;
			GetClientRect(hWnd, &rcPaint);
			paintButton(hWnd, (HDC)wParam, &rcPaint);
		}
		return 0;
		break;
	}
	return CallWindowProc(wndDefHoverProc, hWnd, message, wParam, lParam);
}

// Paints the button with the selected image, the set text and font
void paintButton(HWND hWnd, HDC hdc, RECT *rcPaint)
{
	HoverButton *hoverButton;
	HFONT hFontOld;
	HBITMAP hbmpOld, hbmpImage;
	HDC hdcMem;
	RECT rect;

	// Gets the dimensions of the button
	GetClientRect(hWnd, &rect);
	// Create a DC in memory, compatible with the button's original DC.
	hdcMem = CreateCompatibleDC(hdc);
	// Load the selected image from the resource file.
	hbmpImage = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
	// Select the image into the DC. Keep a reference to the old bitmap.
	hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmpImage);

	hoverButton = findHoverButton(0, hWnd);
	setHoverButtonImage(hoverButton, hdcMem, hoverButton->activeImage);
	if (hoverButton->caption)
	{				
		RECT rcText = {0};
		SIZE size;
		SetBkMode(hdcMem, TRANSPARENT);
		SetTextColor(hdcMem, hoverButton->color);
		hFontOld = (HFONT)SelectObject(hdcMem, hoverButton->hFont);

		// Calculates the size of the rectangle needed to fit the text in. 
		DrawText(hdcMem, hoverButton->caption, (int)_tcslen(hoverButton->caption), &rcText, DT_CALCRECT);
		rectToSize(&rcText, &size);
		// Print the text in the center of the control
		rcText.left = ROUND((hoverButton->buttonRect.right - hoverButton->buttonRect.left - rcText.right) / 2.0f);
		rcText.top = ROUND((hoverButton->buttonRect.bottom - hoverButton->buttonRect.top - rcText.bottom) / 2.0f);
		rcText.right = rcText.right + rcText.left;
		rcText.bottom = rcText.bottom + rcText.top;
		DrawText(hdcMem, hoverButton->caption, (int)_tcslen(hoverButton->caption), &rcText, DT_WORDBREAK | DT_EXPANDTABS | DT_CENTER);
		SelectObject(hdcMem, hFontOld);
	}
	// Copies the bitmap from the memory DC into the buttons DC 
	BitBlt(hdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, hdcMem, rcPaint->left, rcPaint->top, SRCCOPY);
	//BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);
	// Select the original memory DC's bitmap.
	SelectObject(hdcMem, hbmpOld);
	// Free resources.
	DeleteDC(hdcMem);
	DeleteObject(hbmpImage);
}

// Retrieves a pointer to the control from the list of existing controls
HoverButton *findHoverButton(int cId, HWND hWnd)
{
	HoverButton *hoverButton = NULL;

	if (!hoverButtonList)
		return hoverButton;

	for (listSelectFirst(hoverButtonList); listSelectCurrent(hoverButtonList); listSelectNext(hoverButtonList, NULL))
	{
		listGetValue(hoverButtonList, NULL, &hoverButton);
		if (hWnd && hoverButton->hButton == hWnd)
			return hoverButton;
		else if (cId && hoverButton->cId == cId)
			return hoverButton;
	}

	return hoverButton;
}

// Invalidates the HoverButton's Rect to force redraw of the button
void invalidateButtonRect(HoverButton *hoverButton)
{
	RECT rc;
	GetClientRect(hoverButton->hButton, &rc);
	InvalidateRect(hoverButton->hButton, &rc, FALSE);
}