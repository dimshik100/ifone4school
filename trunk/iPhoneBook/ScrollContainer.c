#include "StdAfx.h"
#include "ScrollContainer.h"
#include "HoverButton.h"
#include "Miscellaneous.h"

LRESULT CALLBACK	ScrollContainerProc(HWND, UINT, WPARAM, LPARAM);
WNDPROC wndDefContainerProc = NULL;

int getCurrentScrollPos(HWND hWnd, int scrollBar);
int getScrollPos(HWND hWnd, int scrollBar, UINT code);

HWND createScrollContainer(HWND hWndParent, HINSTANCE hInstance, DWORD dwStyle, int x, int y, int width, int height, int virtWidth, int virtHeight, int controlId, int bgImage)
{
	HoverButton *hoverButton;
	HWND hWndContainer;
	SIZE size, virtSize;

	size.cx = width, size.cy = height;
	virtSize.cx = virtWidth, virtSize.cy = virtHeight;
	hoverButton = createHoverButton(hWndParent, hInstance, x, y, width, height, controlId, bgImage, bgImage, NULL);
	lockHoverButtonImage(hoverButton, TRUE);
	hWndContainer = getHoverButtonHwnd(hoverButton);
	ShowWindow(hWndContainer, SW_HIDE);
	wndDefContainerProc = (WNDPROC)SetWindowLong(hWndContainer, GWL_WNDPROC, (LONG_PTR)ScrollContainerProc);
	SetWindowLong(hWndContainer, GWL_STYLE, GetWindowLong(hWndContainer, GWL_STYLE) | WS_VSCROLL | WS_HSCROLL | dwStyle);
	setScrollContainerSize(hWndContainer, &size, &virtSize);

	return hWndContainer;
}

void setScrollContainerSize(HWND hWnd, SIZE *size, SIZE *virtSize)
{
	SCROLLINFO si;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPos = si.nMin = 1;

	SetWindowPos(hWnd, NULL, 0, 0, size->cx, size->cy, SWP_NOMOVE | SWP_NOOWNERZORDER);

	si.nMax = virtSize->cy;
	si.nPage = size->cy;
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	GetScrollInfo(hWnd, SB_VERT, &si);

	si.nMax = virtSize->cx;
	si.nPage = size->cx;
	SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
	GetScrollInfo(hWnd, SB_HORZ, &si);
}

void setScrollContainerImageStretch(HWND hWnd, int enable)
{
	HoverButton *hoverButton = findButton(0, hWnd);
	if (hoverButton)
		setHoverButtonImageStretch(hoverButton, enable);
}

LRESULT CALLBACK	ScrollContainerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEHWHEEL:
	case WM_MOUSEWHEEL:
		{
			int scrolledRaw = (int)wParam, divider = WHEEL_DELTA;
			int scrolled;
			int orientation = (message == WM_MOUSEWHEEL) ? (1) : (-1);
			int scrollBar = (message == WM_MOUSEWHEEL) ? (SB_VERT) : (SB_HORZ);
			UINT newMsg = (message == WM_MOUSEWHEEL) ? (WM_VSCROLL) : (WM_HSCROLL);

			// Calculate the number of lines scrolled.
			while (scrolledRaw % divider == 0)
				divider += WHEEL_DELTA;
			scrolled = (divider / (scrolledRaw % divider)) * orientation;

			// Set a new scroll position
			SetScrollPos(hWnd, scrollBar, getCurrentScrollPos(hWnd, scrollBar) - (scrolled*10), FALSE);
			// Notify our window a scroll has to occur.
			SendMessage(hWnd, newMsg, SB_LINEUP, (LPARAM)NULL);

			return FALSE;
		}
		break;
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
					shiftChildWindows(hWnd, cx, cy);
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

int getCurrentScrollPos(HWND hWnd, int scrollBar)
{
	SCROLLINFO si;
	// Retrieve current scroll bar value
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	GetScrollInfo(hWnd, scrollBar, &si);
	return si.nPos;
}

int getScrollPos(HWND hwnd, int scrollBar, UINT code)
{
    SCROLLINFO si;
    int minPos;
    int maxPos;
    int result = INT_MAX;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
    GetScrollInfo(hwnd, scrollBar, &si);

	minPos = si.nMin;
	maxPos = si.nMax - (si.nPage - 1);

    switch(code)
    {
    case SB_LINEUP /*SB_LINELEFT*/:
        result = max(si.nPos - 1, minPos);
        break;

    case SB_LINEDOWN /*SB_LINERIGHT*/:
        result = min(si.nPos + 1, maxPos);
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
        result = si.nTrackPos;
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