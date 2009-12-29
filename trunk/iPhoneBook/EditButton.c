#include "StdAfx.h"
#include "EditButton.h"

WNDPROC hDefEditBtnProc = NULL;

EditButton *createEditButton(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height, int controlId, WNDPROC newWndProc)
{
	EditButton *newEditButton = (EditButton*)calloc(1, sizeof(EditButton));

	newEditButton->hInstance = hInstance;
	newEditButton->buttonRect.left = x;
	newEditButton->buttonRect.right = x + width;
	newEditButton->buttonRect.top = y;
	newEditButton->buttonRect.bottom = y + height;
	newEditButton->editRect = newEditButton->buttonRect;
	newEditButton->hButton = CreateWindowEx(0, TEXT("button"), NULL, WS_VISIBLE | WS_CHILD, x, y, width, height,
		hWndParent, (HMENU)controlId, hInstance, NULL);

	x += 20;
	width -= -(20 + 40);
	y += 10;
	height -= -(10 + 10);
	newEditButton->buttonRect.left = x;
	newEditButton->buttonRect.right = x + width;
	newEditButton->buttonRect.top = y;
	newEditButton->buttonRect.bottom = y + height;
	newEditButton->hEdit = CreateWindowEx(0, TEXT("edit"), NULL, WS_CHILD,
		x, y, width, height,
		newEditButton->hButton, (HMENU)(controlId+1), hInstance, NULL);

	/// !!! Apply new window procedure to control !!! ///
	
	return newEditButton;
}

void setEditButtonImage(EditButton *editButton, HDC hDC, int imageId)
{
	HBITMAP hbmpOld, hbmpImage;
	HDC hDCMem;
	RECT rect;

	// Create a DC in memory, compatible with the button's original DC.
	hDCMem = CreateCompatibleDC(hDC);
	// Load the selected image from the resource file.
	hbmpImage = LoadBitmap(editButton->hInstance, MAKEINTRESOURCE(imageId));
	// Select the image into the DC. Keep a reference to the old bitmap.
	hbmpOld = (HBITMAP)SelectObject(hDCMem, hbmpImage);
	// Gets the dimensions of the button
	GetClientRect(editButton->hButton, &rect);
	// Copies the bitmap from the memory DC into the buttons DC 
	BitBlt(hDC, 0, 0, rect.right, rect.bottom, hDCMem, 0, 0, SRCCOPY);
	// Select the original memory DC's bitmap.
	SelectObject(hDCMem, hbmpOld);
	// Free resources.
	DeleteDC(hDCMem);
	DeleteObject(hbmpImage);
}

void setEditButtonText(EditButton *editButton, HDC hDC, TCHAR *string)
{
}

void setEditButtonFont(EditButton *editButton, TCHAR *fontName, int fontSize)
{
}

void showEditButtonEdit(EditButton *editButton, int show)
{
	ShowWindow(editButton->hEdit, show);
	ShowWindow(editButton->hEdit, show);
	editButton->inTextMode = show;
}