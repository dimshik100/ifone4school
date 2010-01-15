#include "StdAfx.h"
#include "Miscellaneous.h"


BOOL CALLBACK		shiftChildWindowsProc(HWND hWnd, LPARAM lParam);
BOOL CALLBACK		showChildWindowsEnumProc(HWND hWnd, LPARAM lParam);

BOOL CALLBACK		shiftChildWindowsProc(HWND hWnd, LPARAM lParam)
{
	void **paramArray = (void**)lParam;
	HWND hWndParent = *(HWND*)paramArray[0];

	// Make sure that we only move the first-children and not any other nested children.
	if (hWndParent == GetParent(hWnd))
	{
		RECT rc;
		POINT ptCtl = {0, 0} , ptParent = {0, 0}, ptOffset;
		ptOffset.x = *(int*)paramArray[1], ptOffset.y = *(int*)paramArray[2];

		ClientToScreen(GetParent(hWnd), &ptParent);
		ClientToScreen(hWnd, &ptCtl);
		GetClientRect(hWnd, &rc);
		ptCtl.x -= ptParent.x;
		ptCtl.y -= ptParent.y;
		MoveWindow(hWnd, ptCtl.x + ptOffset.x , ptCtl.y + ptOffset.y, rc.right, rc.bottom, TRUE);
	}

	return TRUE;
}

void shiftChildWindows(HWND hWnd, int xOffset, int yOffset)
{
	void *paramArray[3];

	paramArray[0] = &hWnd;
	paramArray[1] = &xOffset;
	paramArray[2] = &yOffset;
	EnumChildWindows(hWnd, shiftChildWindowsProc, (LPARAM)paramArray);

}

BOOL CALLBACK		showChildWindowsEnumProc(HWND hWnd, LPARAM lParam)
{
	ShowWindow(hWnd, (int)lParam);
	return TRUE;
}

void showChildWindows(HWND hWnd, int nCmdShow)
{
	EnumChildWindows(hWnd, showChildWindowsEnumProc, (LPARAM)nCmdShow);
}

DynamicListC getCurrentContactList(int fromFile);

DynamicListC getContactListLocal()
{
	return getCurrentContactList(FALSE);
}

DynamicListC getContactListFromFile()
{
	return getCurrentContactList(TRUE);
}

DynamicListC getContactListInitiated()
{
	DynamicListC list = getCurrentContactList(FALSE);
	if (!list)
		list = getCurrentContactList(TRUE);
	return list;
}

DynamicListC getCurrentContactList(int fromFile)
{
	static DynamicListC contactList = NULL;

	if (fromFile)
	{	
		if (contactList)
			listFree(&contactList);
		//contactList = getContactList();
	}

	return contactList;
}

void setImageToDcActual(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage, int stretch);

void setBitmapToDc(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage)
{
	setImageToDcActual(lprc, lprcOffset, hdc, hbmpImage, FALSE);
}

void setBitmapToDcStretched(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage)
{
	setImageToDcActual(lprc, lprcOffset, hdc, hbmpImage, TRUE);
}

void setImageToDc(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId)
{
	// Load the selected image from the resource file.
	HBITMAP hbmpImage = LoadBitmap(hInstance, MAKEINTRESOURCE(imageId));
	setImageToDcActual(lprc, lprcOffset, hdc, hbmpImage, FALSE);
	DeleteObject(hbmpImage);
}

void setImageToDcStretched(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId)
{
	// Load the selected image from the resource file.
	HBITMAP hbmpImage = LoadBitmap(hInstance, MAKEINTRESOURCE(imageId));
	setImageToDcActual(lprc, lprcOffset, hdc, hbmpImage, TRUE);
	DeleteObject(hbmpImage);
}

void setImageToDcActual(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage, int stretch)
{
	HBITMAP hbmpOld;
	BITMAP bm;
	HDC hdcMem;

	// Create a DC in memory, compatible with the button's original DC.
	hdcMem = CreateCompatibleDC(hdc);
	// Select the image into the DC. Keep a reference to the old bitmap.
	hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmpImage);
	if (stretch)
	{
		// Copies the bitmap from the memory DC into the buttons DC in stretched mode
		int stretchMode = GetStretchBltMode(hdc);
		// Get bitmap size
		GetObject(hbmpImage, sizeof(bm), &bm);
		// Set stretch mode, draw image, set stretch mode back to original
		SetStretchBltMode(hdc, COLORONCOLOR);
		StretchBlt(hdc, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top, hdcMem, lprcOffset->left, lprcOffset->top, bm.bmWidth, bm.bmHeight, SRCCOPY);
		SetStretchBltMode(hdc, stretchMode);
	}
	else
		// Copies the bitmap from the memory DC into the buttons DC 
		BitBlt(hdc, lprc->left, lprc->top, lprc->right, lprc->bottom, hdcMem, lprcOffset->left, lprcOffset->top, SRCCOPY);
	// Select the original memory DC's bitmap.
	SelectObject(hdcMem, hbmpOld);
	// Free resources.
	DeleteDC(hdcMem);
}

int isOsVista()
{
	typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);
	PROCNTQSI NtQuerySystemInformation;
	OSVERSIONINFO osv;

	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
		GetModuleHandle(_T("ntdll")), "NtQuerySystemInformation");
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx( &osv )) {
		if (osv.dwPlatformId == 2 && osv.dwMajorVersion >= 6)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}