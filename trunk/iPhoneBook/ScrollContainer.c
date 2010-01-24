#include "StdAfx.h"
#include "ScrollContainer.h"
#include "HoverButton.h"
#include "Miscellaneous.h"

LRESULT CALLBACK	ScrollContainerProc(HWND, UINT, WPARAM, LPARAM);
WNDPROC wndDefContainerProc = NULL;

int getCurrentScrollPos(HWND hWnd, int scrollBar);
int getScrollPos(HWND hWnd, int scrollBar, UINT code);

///Temporarily global variables:
SIZE itemSize;

HWND createScrollContainer(HWND hWndParent, HINSTANCE hInstance, DWORD dwStyle, int x, int y, int width, int height, int itemWidth, int itemHeight, int virtWidth, int virtHeight, int controlId, int bgImage)
{
	HoverButton *hoverButton;
	HWND hWndContainer;
	SIZE size, virtSize;

	size.cx = width, size.cy = height;
	virtSize.cx = virtWidth, virtSize.cy = virtHeight;
	itemSize.cx = itemWidth, itemSize.cy = itemHeight;
	hoverButton = createHoverButton(hWndParent, hInstance, x, y, width, height, controlId, bgImage, bgImage, NULL);
	lockHoverButtonImage(hoverButton, TRUE);
	hWndContainer = getHoverButtonHwnd(hoverButton);
	wndDefContainerProc = (WNDPROC)SetWindowLong(hWndContainer, GWL_WNDPROC, (LONG_PTR)ScrollContainerProc);
	SetWindowLong(hWndContainer, GWL_STYLE, GetWindowLong(hWndContainer, GWL_STYLE) | WS_VSCROLL | WS_HSCROLL | dwStyle);
	// Adjust the size of the control and the size of the scroll area.
	setScrollContainerSize(hWndContainer, &size, &size);
	setScrollContainerSize(hWndContainer, &size, &virtSize);

	return hWndContainer;
}

// Changes the physical dimentions of the control and adjusts the scroll area size.
void setScrollContainerSize(HWND hWnd, SIZE *size, SIZE *virtSize)
{
	SCROLLINFO si = {0};
	POINT pt;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	si.nPos = si.nMin = 1;

	si.nMax = virtSize->cx;
	si.nPage = size->cx;
	SetScrollInfo(hWnd, SB_HORZ, &si, FALSE);

	si.fMask |= SIF_DISABLENOSCROLL;
	si.nMax = virtSize->cy;
	si.nPage = size->cy;
	SetScrollInfo(hWnd, SB_VERT, &si, FALSE);

	getChildInParentOffset(hWnd, &pt);
	MoveWindow(hWnd, pt.x, pt.y, size->cx, size->cy, TRUE);
	// Sends a message simulating clicks on the scroll bar's arrows
	// to adjust the position of the scroll bar in the control
	SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, (LPARAM)NULL);
	SendMessage(hWnd, WM_HSCROLL, SB_LINEUP, (LPARAM)NULL);
}

// Procedure to handle messages for this control
LRESULT CALLBACK	ScrollContainerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	// On resize adjust the dimentions of the scroll area and scrollbar page sizes.
	case WM_SIZE:
		{
			int cx = LOWORD(lParam);
			int cy = HIWORD(lParam);
			SCROLLINFO si = {0};

			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_PAGE;
			si.nPage = cx;
			SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
			si.fMask |= SIF_DISABLENOSCROLL;
			si.nPage = cy;
			SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		}
		break;
	// When the mouse wheel is turned on the control scroll it.
	case WM_MOUSEHWHEEL:
	case WM_MOUSEWHEEL:
		{
			int scrolledRaw = (int)wParam, divider = WHEEL_DELTA;
			int scrolled, i;
			int direction;
			int orientation = (message == WM_MOUSEWHEEL) ? (1) : (-1);
			UINT newMsg = (message == WM_MOUSEWHEEL) ? (WM_VSCROLL) : (WM_HSCROLL);

			// Calculate the number of lines scrolled.
			while (scrolledRaw % divider == 0)
				divider += WHEEL_DELTA;
			scrolled = (divider / (scrolledRaw % divider)) * orientation;
			direction = (scrolled < 0) ? (SB_LINEDOWN) : (SB_LINEUP);

			// Set a new scroll position
			for (i = 0; i < abs(scrolled); i++)
				// Notify our window a scroll has to occur.
				SendMessage(hWnd, newMsg, direction, (LPARAM)NULL);

			return FALSE;
		}
		break;
	// When manually scrolling the bar. Calculate the new position.
	case WM_VSCROLL:
	case WM_HSCROLL:
		{
			static int prevY = 1;
			static int prevX = 1;
			const int scrollBar = (message == WM_VSCROLL) ? (SB_VERT) : (SB_HORZ);

			int scrollPos = getScrollPos(hWnd, scrollBar, LOWORD(wParam));
			if (scrollPos < INT_MAX)
			{
				int cx = 0, cy = 0;
				int *deltaPos = (message == WM_VSCROLL) ? (&cy) : (&cx);
				int *prevVal = (message == WM_VSCROLL) ? (&prevY) : (&prevX);

				*deltaPos = *prevVal - scrollPos;
				if (*deltaPos != 0)
				{
					SetScrollPos(hWnd, scrollBar, scrollPos, TRUE);
					shiftChildWindows(hWnd, cx, cy, TRUE);
					InvalidateRect(hWnd, NULL, FALSE);
					*prevVal = scrollPos;
				}
			}
			else
				return FALSE;
		}
		break;
	}

	return CallWindowProc(wndDefContainerProc, hWnd, message, wParam, lParam);
}

// Get the current position of the specified scroll bar
int getCurrentScrollPos(HWND hWnd, int scrollBar)
{
	SCROLLINFO si;
	// Retrieve current scroll bar value
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	GetScrollInfo(hWnd, scrollBar, &si);
	return si.nPos;
}

// Gets the new position the scrollbar should be set to
int getScrollPos(HWND hwnd, int scrollBar, UINT code)
{
    SCROLLINFO si;
    int minPos;
    int maxPos;
	int item;
    int result = INT_MAX;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
    GetScrollInfo(hwnd, scrollBar, &si);

	minPos = si.nMin;
	maxPos = si.nMax - (si.nPage - 1);
	item = (scrollBar == SB_VERT) ? (itemSize.cy) : (itemSize.cx);

    switch(code)
    {
    case SB_LINEUP /*SB_LINELEFT*/:
        result = max(si.nPos - item, minPos);
        break;

    case SB_LINEDOWN /*SB_LINERIGHT*/:
        result = min(si.nPos + item, maxPos);
        break;

    case SB_PAGEUP /*SB_PAGELEFT*/:
        result = max(si.nPos - (int)si.nPage, minPos);
        break;

    case SB_PAGEDOWN /*SB_PAGERIGHT*/:
        result = min(si.nPos + (int)si.nPage, maxPos);
        break;

    case SB_THUMBPOSITION:
        // do nothing
        break;

    case SB_THUMBTRACK:
		result = (si.nTrackPos < maxPos) ? ((si.nTrackPos / item) * item) + 1: maxPos;
        break;

    case SB_TOP /*SB_LEFT*/:
        result = minPos;
        break;

    case SB_BOTTOM /*SB_RIGHT*/:
        result = maxPos;
        break;

    case SB_ENDSCROLL:
        // do nothing
        break;
    }

    return result;
}

// Write a text on the background center of the scroll control
void setScrollContainerText(HWND hWnd, TCHAR *caption)
{
	HoverButton *hoverButton = findHoverButton(0, hWnd);
	if (hoverButton)
	{
		setHoverButtonText(hoverButton, caption);
	}
}

// Gets the text from the scroll control's background
void getScrollContainerText(HWND hWnd, TCHAR *destination, size_t length)
{
	HoverButton *hoverButton = findHoverButton(0, hWnd);
	if (hoverButton)
		getHoverButtonText(hoverButton, destination, length);
}

// Set an image background on scroll control
void setScrollContainerImage(HWND hWnd, int imgImage)
{
	HoverButton *hoverButton = findHoverButton(0, hWnd);
	if (hoverButton)
	{
		setHoverButtonStateImages(hoverButton, imgImage, imgImage);
	}
}

// Enable/Disable stretching of the background image on the control
void setScrollContainerImageStretch(HWND hWnd, int enable)
{
	HoverButton *hoverButton = findHoverButton(0, hWnd);
	if (hoverButton)
	{
		setHoverButtonImageStretch(hoverButton, enable);
	}
}

// Set a new font to the control
void setScrollContainerFont(HWND hWnd, TCHAR *fontName, int fontSize, int isBold)
{
	HoverButton *hoverButton = findHoverButton(0, hWnd);
	if (hoverButton)
	{
		setHoverButtonFont(hoverButton, fontName, fontSize, isBold);
	}
}

// Returns the currently selected font in the scroll control
HFONT getScrollContainerFont(HWND hWnd)
{
	HoverButton *hoverButton = findHoverButton(0, hWnd);
	HFONT hFont = NULL;
	if (hoverButton)
	{
		hFont = getHoverButtonFont(hoverButton);
	}
	return hFont;
}

// Set text color for the scroll control
void setScrollContainerTextColor(HWND hWnd, COLORREF color)
{
	HoverButton *hoverButton = findHoverButton(0, hWnd);
	if (hoverButton)
	{
		setHoverButtonTextColor(hoverButton, color);
	}
}