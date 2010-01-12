#include "StdAfx.h"
#include "Miscellaneous.h"

void setImageToDC(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hDC, int imageId)
{
	HBITMAP hbmpOld, hbmpImage;
	HDC hDCMem;

	// Create a DC in memory, compatible with the button's original DC.
	hDCMem = CreateCompatibleDC(hDC);
	// Load the selected image from the resource file.
	hbmpImage = LoadBitmap(hInstance, MAKEINTRESOURCE(imageId));
	// Select the image into the DC. Keep a reference to the old bitmap.
	hbmpOld = (HBITMAP)SelectObject(hDCMem, hbmpImage);
	// Copies the bitmap from the memory DC into the buttons DC 
	BitBlt(hDC, lprc->left, lprc->top, lprc->right, lprc->bottom, hDCMem, lprcOffset->left, lprcOffset->top, SRCCOPY);
	// Select the original memory DC's bitmap.
	SelectObject(hDCMem, hbmpOld);
	// Free resources.
	DeleteDC(hDCMem);
	DeleteObject(hbmpImage);
}