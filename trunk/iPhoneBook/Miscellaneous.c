#include "StdAfx.h"
#include "Miscellaneous.h"

DWORD WINAPI		AnimateWindowThreadProc( LPVOID lpParam );
BOOL CALLBACK		shiftChildWindowsProc(HWND hWnd, LPARAM lParam);
BOOL CALLBACK		invalidateChildWindowsProc(HWND hWnd, LPARAM lParam);
BOOL CALLBACK		showChildWindowsEnumProc(HWND hWnd, LPARAM lParam);
BOOL CALLBACK		setChildWindowsFontProc(HWND hWnd, LPARAM lParam);
void setImageToDcActual(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage, int stretch);

// Calculates the position of a child-window inside it's parent.
void getChildInParentOffset(HWND hWnd, POINT *lppt)
{
	POINT ptCtl = {0, 0} , ptParent = {0, 0};

	ClientToScreen(GetParent(hWnd), &ptParent);
	ClientToScreen(hWnd, &ptCtl);
	lppt->x = ptCtl.x - ptParent.x;
	lppt->y = ptCtl.y - ptParent.y;
}

// EnumChildWindows callback. Moves all first degree child windows by a specified offset
BOOL CALLBACK		shiftChildWindowsProc(HWND hWnd, LPARAM lParam)
{
	void **paramArray = (void**)lParam;
	HWND hWndParent = *(HWND*)paramArray[0];

	// Make sure that we only move the first-children and not any other nested children.
	if (hWndParent == GetParent(hWnd))
	{
		RECT rc;
		POINT ptCtl, ptOffset;
		BOOL bErase = *(BOOL*)paramArray[3];
		ptOffset.x = *(int*)paramArray[1], ptOffset.y = *(int*)paramArray[2];

		getChildInParentOffset(hWnd, &ptCtl);
		GetClientRect(hWnd, &rc);
		MoveWindow(hWnd, ptCtl.x + ptOffset.x , ptCtl.y + ptOffset.y, rc.right, rc.bottom, FALSE);
		InvalidateRect(hWnd, NULL, bErase);
	}

	return TRUE;
}

void shiftChildWindows(HWND hWnd, int xOffset, int yOffset, BOOL bErase)
{
	void *paramArray[4];

	paramArray[0] = &hWnd;
	paramArray[1] = &xOffset;
	paramArray[2] = &yOffset;
	paramArray[3] = &bErase;
	EnumChildWindows(hWnd, shiftChildWindowsProc, (LPARAM)paramArray);
}

// EnumChildWindows callback. Invalidates all child windows of a parent.
BOOL CALLBACK		invalidateChildWindowsProc(HWND hWnd, LPARAM lParam)
{
	InvalidateRect(hWnd, NULL, (BOOL)lParam);
	return TRUE;
}

void invalidateChildWindows(HWND hWnd, BOOL bErase)
{
	EnumChildWindows(hWnd, invalidateChildWindowsProc, (LPARAM)bErase);
}

// EnumChildWindows callback. Updates all child windows of a parent.
BOOL CALLBACK		updateChildWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (GetParent(hWnd) == (HWND)lParam)
		UpdateWindow(hWnd);
	return TRUE;
}

void updateChildWindows(HWND hWnd)
{
	EnumChildWindows(hWnd, invalidateChildWindowsProc, (LPARAM)hWnd);
}

// EnumChildWindows callback. Shows or hides child windows of a parent.
BOOL CALLBACK		showChildWindowsEnumProc(HWND hWnd, LPARAM lParam)
{
	ShowWindow(hWnd, (int)lParam);
	return TRUE;
}

void showChildWindows(HWND hWnd, int nCmdShow)
{
	EnumChildWindows(hWnd, showChildWindowsEnumProc, (LPARAM)nCmdShow);
}

// Draws a bitmap on the DC
void setBitmapToDc(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage)
{
	setImageToDcActual(lprc, lprcOffset, hdc, hbmpImage, FALSE);
}

// Draws a bitmap on the DC in stretched mode (fits the lprc parameter)
void setBitmapToDcStretched(RECT *lprc, RECT *lprcOffset, HDC hdc, HBITMAP hbmpImage)
{
	setImageToDcActual(lprc, lprcOffset, hdc, hbmpImage, TRUE);
}

// Draws a resource image on the DC
void setImageToDc(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId)
{
	// Load the selected image from the resource file.
	HBITMAP hbmpImage = LoadBitmap(hInstance, MAKEINTRESOURCE(imageId));
	setImageToDcActual(lprc, lprcOffset, hdc, hbmpImage, FALSE);
	DeleteObject(hbmpImage);
}

// Draws a resource image on the DC in stretched mode (fits the lprc parameter)
void setImageToDcStretched(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId)
{
	// Load the selected image from the resource file.
	HBITMAP hbmpImage = LoadBitmap(hInstance, MAKEINTRESOURCE(imageId));
	setImageToDcActual(lprc, lprcOffset, hdc, hbmpImage, TRUE);
	DeleteObject(hbmpImage);
}

// Draws the supplied bitmap on the supplied DC in the specified format.
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

// Checks if the OS in use is at least Vista or newer (supports new visual features)
int isOsVista()
{
	typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);
	PROCNTQSI NtQuerySystemInformation;
	OSVERSIONINFO osv;

	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
		GetModuleHandle(TEXT("ntdll")), "NtQuerySystemInformation");
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx( &osv )) {
		if (osv.dwPlatformId == 2 && osv.dwMajorVersion >= 6)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

// Converts a RECT into a SIZE structure
void rectToSize(RECT *rc, SIZE *size)
{
	if (rc && size)
	{
		size->cx = rc->right - rc->left;
		size->cy = rc->bottom - rc->top;
	}
}

// Displays a part of the window by using a mask bitmap. 'maskRect' may be NULL, function will ignore if it is NULL.
void makeWindowTransparentByMask(HWND hWnd, RECT *maskRect, int mask)
{
	HBITMAP hbmp, hbmpOld;
	BITMAP bm;
	HDC hdcMem, hdc;
	BITMAPINFOHEADER bmInfo = {0};
	COLORREF *bits;
	POINT pt;
	RECT rc, rcRgn = {0}, maskOffsetRc = {0};
	HRGN hrgn, hrgnTemp;
	int x, y;

	// Create memory DC
	hdc = GetDC(hWnd);
	hdcMem = CreateCompatibleDC(hdc);
	ReleaseDC(hWnd, hdc);
	
	// Load the mask bitmap from the resource file
	hbmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(mask));
	hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmp);
	// Get the bitmap size
	GetObject(hbmp, sizeof(bm), &bm);
	// Fill in the required data for GetDIBits to work.
	bmInfo.biSize = sizeof(BITMAPINFOHEADER);    
	bmInfo.biWidth = bm.bmWidth;    
	bmInfo.biHeight = -bm.bmHeight;  // Scan BMP from top to bottom
	bmInfo.biPlanes = 1;    
	bmInfo.biBitCount = 32;    
	bmInfo.biCompression = BI_RGB;
	// Get the memory size required to load the image
	GetDIBits(hdcMem, hbmp, 0, bm.bmHeight, NULL, (LPBITMAPINFO)&bmInfo, DIB_RGB_COLORS);
	bits = (COLORREF*)malloc(bmInfo.biSizeImage);
	// Load image bits into memory
	GetDIBits(hdcMem, hbmp, 0, bm.bmHeight, bits, (LPBITMAPINFO)&bmInfo, DIB_RGB_COLORS);

	GetClientRect(hWnd, &rc);
	AdjustWindowRect(&rc, GetWindowLong(hWnd, GWL_STYLE) - WS_OVERLAPPED, FALSE);
	// User provided offset from Left-Top and from Right-Bottom of rect.
	if (maskRect)
		memcpy(&maskOffsetRc, maskRect, sizeof(RECT));
	// Calculate the size of the initial region.
	rcRgn.left = maskOffsetRc.left - rc.left;
	rcRgn.top = maskOffsetRc.top - rc.top;
	rcRgn.right = rcRgn.left + bm.bmWidth - maskOffsetRc.left - maskOffsetRc.right;
	rcRgn.bottom = rcRgn.top + bm.bmHeight - maskOffsetRc.top - maskOffsetRc.bottom;

	getChildInParentOffset(hWnd, &pt);
	hrgn = CreateRectRgn(rcRgn.left, rcRgn.top, rcRgn.right, rcRgn.bottom);
	// Loop through the image "rect" and remove all off-pixels from our initial region.
	for (x = rcRgn.left; x <  rcRgn.right; x++)
	{
		for (y = rcRgn.top; y < rcRgn.bottom; y++)
		{
			if (!bits[(y+rc.top)*bm.bmWidth+(x+rc.left)])
			{
				hrgnTemp = CreateRectRgn(x, y, x+1, y+1);
				CombineRgn(hrgn, hrgn, hrgnTemp, RGN_XOR);
				DeleteObject(hrgnTemp);
			}
		}
	}
	// Apply the region to our window
	SetWindowRgn(hWnd, hrgn, TRUE);
	// Free resources.
	free(bits);

	SelectObject(hdc, hbmpOld);
	DeleteObject(hbmp);
	DeleteDC(hdcMem);
}

// Gets the path to where the program's executable file lays.
void getModulePath(LPWCH lpFilename, DWORD nSize)
{
	TCHAR *ptrEnd;	
	GetModuleFileName(NULL, lpFilename, nSize);			
	ptrEnd = _tcsrchr(lpFilename, TEXT('\\'));
	if (ptrEnd)
		*(ptrEnd+1) = TEXT('\0');
}

// Loads a custom font by filename
int loadCustomFont(LPTSTR fontFileName)
{
	TCHAR fullPath[MAX_PATH];
	getModulePath(fullPath, MAX_PATH);
	_tcscat_s(fullPath, MAX_PATH, fontFileName);
	CreateScalableFontResource(1, fullPath, fullPath, NULL);
	return AddFontResourceW(fullPath);
}

// Unloads a custom font by filename
BOOL unloadCustomFont(LPTSTR fontFileName)
{
	TCHAR fullPath[MAX_PATH];
	getModulePath(fullPath, MAX_PATH);
	_tcscat_s(fullPath, MAX_PATH, fontFileName);
	return RemoveFontResource(fullPath);
}

// Calculates the font height based on the window's DC 
// and the provided font point-size
int getFontHeight(HWND hWnd, int ptSize)
{
	HDC hdc =  GetDC(hWnd);
	int fontHeight = -MulDiv(ptSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(hWnd, hdc);
	return fontHeight;
}

// EnumChildWindows callback. Applies the provided font to all child windows of a parent.
BOOL CALLBACK		setChildWindowsFontProc(HWND hWnd, LPARAM lParam)
{
	SendMessage(hWnd, WM_SETFONT, (WPARAM)(HFONT)lParam, (LPARAM)TRUE);
	return TRUE;
}

void setChildWindowsFont(HWND hWnd, HFONT hFont)
{
	EnumChildWindows(hWnd, setChildWindowsFontProc, (LPARAM)hFont);
}