#include "StdAfx.h"
#include "Clock.h"
#include "Miscellaneous.h"
#include <math.h>

LRESULT CALLBACK	ClockProc(HWND, UINT, WPARAM, LPARAM);
void DrawBasicClock(HWND hWnd);
WNDPROC clockDefProc = NULL;

HWND createClock(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height, int controlId, int image)
{
	HWND hwndClock;

	hwndClock = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP, x, y, width, height, hWndParent, (HMENU)controlId, hInstance, NULL);
	clockDefProc = (WNDPROC)SetWindowLong(hwndClock, GWLP_WNDPROC, (LONG_PTR)ClockProc);
	if (image)
		SendMessage(hwndClock, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(hInstance, MAKEINTRESOURCE(image)));
	SetTimer(hwndClock, 0, 500, NULL);

	return hwndClock;
}

LRESULT CALLBACK	ClockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_TIMER:
		DrawBasicClock(hWnd);
		break;
	case WM_DESTROY:
		{
			// If we had set a background image to our container, we should free it's memory/resources.
			HBITMAP hBmp;
			hBmp = (HBITMAP)SendMessage(hWnd, STM_GETIMAGE, IMAGE_BITMAP, (LPARAM)0);
			if (hBmp)
				DeleteObject(hBmp);
		}
		break;
	}

	return CallWindowProc(clockDefProc, hWnd, message, wParam, lParam);
}

void DrawBasicClock(HWND hWnd)
{
	static const double pi = 3.1415926535897932384626433832795, degreeMultiplier[3] = { 6, 6, 30 };
	time_t ltime;
	struct tm today;
	double degree;
	POINT pt[3];
	RECT rc;
	int i;
	LONG centerX, centerY, maxArmLen, armLens[3];
	HDC hdc;

	GetClientRect(hWnd, &rc);
	centerX = rc.right / 2;
	centerY = rc.bottom / 2;
	maxArmLen = min(centerX, centerY);
	time(&ltime);
	if (localtime_s(&today, &ltime) == ERROR_SUCCESS)
	{
		armLens[0] = (maxArmLen * 4) / 5;
		degree = pi - ((degreeMultiplier[0] * today.tm_sec) * (pi / 180));
		pt[0].x = (LONG)(armLens[0] * sin(degree)) + centerX;
		pt[0].y = (LONG)(armLens[0] * cos(degree)) + centerY;
		armLens[1] = (maxArmLen * 3) / 5;
		degree = pi - ((degreeMultiplier[1] * today.tm_min + ((degreeMultiplier[1] * today.tm_sec) / 60)) * (pi / 180));
		pt[1].x = (LONG)(armLens[1] * sin(degree)) + centerX;
		pt[1].y = (LONG)(armLens[1] * cos(degree)) + centerY;
		armLens[2] = maxArmLen / 2;
		degree = pi - ((degreeMultiplier[2] * today.tm_hour + ((degreeMultiplier[2] * today.tm_min) / 60) + ((degreeMultiplier[2] * today.tm_sec) / 3600)) * (pi / 180));
		pt[2].x = (LONG)(armLens[2] * sin(degree)) + centerX;
		pt[2].y = (LONG)(armLens[2] * cos(degree)) + centerY;
	}
	hdc = GetDC(hWnd);
	for (i = 0; i < 3; i++)
	{
		MoveToEx(hdc, centerX, centerY, NULL);
		LineTo(hdc, pt[i].x, pt[i].y);
	}
	ReleaseDC(hWnd, hdc);
}