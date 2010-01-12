#include "StdAfx.h"
#include "Clock.h"

LRESULT CALLBACK	ClockProc(HWND, UINT, WPARAM, LPARAM);
WNDPROC clockDefProc = NULL;

HWND createClock(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height, int controlId, int image)
{
	HWND hwndClock;

	hwndClock = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP, x, y, width, height, hWndParent, (HMENU)controlId, hInstance, NULL);
	if (image)
		SendMessage(hwndClock, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(hInstance, MAKEINTRESOURCE(image)));

	clockDefProc = (WNDPROC)SetWindowLong(hwndClock, GWLP_WNDPROC, (LONG_PTR)ClockProc);

	return hwndClock;
}

LRESULT CALLBACK	ClockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		SetTimer(hWnd, 0, 500, NULL);
		break;
	case WM_TIMER:
		break;
	case WM_DESTROY:
		{
			HBITMAP hBmp;
			hBmp = (HBITMAP)SendMessage(hWnd, STM_GETIMAGE, IMAGE_BITMAP, (LPARAM)0);
			if (hBmp)
				DeleteObject(hBmp);
		}
		break;
	}

	return CallWindowProc(clockDefProc, hWnd, message, wParam, lParam);
}