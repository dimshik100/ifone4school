#include "StdAfx.h"
#include "Miscellaneous.h"
#include "PhoneBook.h"

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
		contactList = getContactList();
	}

	return contactList;
}

void setImageToDcActual(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId, int stretch);

void setImageToDc(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId)
{
	setImageToDcActual(hInstance, lprc, lprcOffset, hdc, imageId, FALSE);
}

void setImageToDcStretched(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId)
{
	setImageToDcActual(hInstance, lprc, lprcOffset, hdc, imageId, TRUE);
}

void setImageToDcActual(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId, int stretch)
{
	HBITMAP hbmpOld, hbmpImage;
	BITMAP bm;
	HDC hdcMem;

	// Create a DC in memory, compatible with the button's original DC.
	hdcMem = CreateCompatibleDC(hdc);
	// Load the selected image from the resource file.
	hbmpImage = LoadBitmap(hInstance, MAKEINTRESOURCE(imageId));
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
	DeleteObject(hbmpImage);
}