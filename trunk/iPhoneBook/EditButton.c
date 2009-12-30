#include "StdAfx.h"
#include "EditButton.h"

#define EDIT_BUTTON_CTL_ID	5000
#define FUNC_BUTTON_SIZE	20
enum {	CID_TEXT_OFFSET	= 1, CID_OK_OFFSET = 2, CID_CANCEL_OFFSET = 3 } ;

WNDPROC wndDefEditBtnProc = NULL;
WNDPROC wndDefBtnProc = NULL;
WNDPROC wndDefEditProc = NULL;

LRESULT CALLBACK	EditBtnProc(HWND, UINT, WPARAM, LPARAM);

EditButton *createEditButton(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height, int controlId)
{
	EditButton *newEditButton = (EditButton*)calloc(1, sizeof(EditButton));
	controlId = EDIT_BUTTON_CTL_ID + (controlId * 10);

	newEditButton->hInstance = hInstance;
	newEditButton->buttonRect.left = x;
	newEditButton->buttonRect.right = x + width;
	newEditButton->buttonRect.top = y;
	newEditButton->buttonRect.bottom = y + height;
	newEditButton->editRect = newEditButton->buttonRect;
	newEditButton->hButton = CreateWindowEx(0, TEXT("button"), NULL, WS_VISIBLE | WS_CHILD, x, y, width, height,
		hWndParent, (HMENU)controlId, hInstance, NULL);

	x = width - (FUNC_BUTTON_SIZE + 5);
	y = height - (FUNC_BUTTON_SIZE + 5);
	newEditButton->hCancel = CreateWindowEx(0, TEXT("button"), NULL, WS_CHILD | BS_OWNERDRAW,
		x, y, FUNC_BUTTON_SIZE, FUNC_BUTTON_SIZE,
		newEditButton->hButton, (HMENU)(controlId+CID_CANCEL_OFFSET), hInstance, NULL);

	x = width - (FUNC_BUTTON_SIZE + 5);
	y = 5;
	newEditButton->hOK = CreateWindowEx(0, TEXT("button"), NULL, WS_CHILD | BS_OWNERDRAW,
		x, y, FUNC_BUTTON_SIZE, FUNC_BUTTON_SIZE,
		newEditButton->hButton, (HMENU)(controlId+CID_CANCEL_OFFSET), hInstance, NULL);

	// Subtract from (left + right);
	width -= (20 + 30);
	// Subtract from (top + bottom);
	height -= (5 + 5);
	newEditButton->editRect.left = 20;
	newEditButton->editRect.right = 20 + width;
	newEditButton->editRect.top = 5;
	newEditButton->editRect.bottom = 5 + height;
	newEditButton->hEdit = CreateWindowEx(0, TEXT("edit"), NULL, WS_CHILD,
		x, y, width, height,
		newEditButton->hButton, (HMENU)(controlId+CID_TEXT_OFFSET), hInstance, NULL);

	/// !!! Apply new window procedure to control !!! ///
	wndDefBtnProc = (WNDPROC)SetWindowLong(newEditButton->hButton, GWL_WNDPROC, (LONG_PTR)EditBtnProc);
	wndDefEditProc = (WNDPROC)SetWindowLong(newEditButton->hEdit, GWL_WNDPROC, (LONG_PTR)EditBtnProc);

	return newEditButton;
}

void setDefaultEditButtonProc(WNDPROC wndProc)
{
	wndDefEditBtnProc = wndProc;
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

LRESULT CALLBACK	EditBtnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC wndDefault = NULL;
	switch (message)
	{		
		
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;

			// Make sure that our control is a button
			if (lpDrawItem->CtlType == ODT_BUTTON)
			{
				// If the button has focus or is highlighted, draw load the "Active" image.
				// Else load the "Inactive" image.
				if (((lpDrawItem->itemState & ODS_FOCUS) || (lpDrawItem->itemState & ODS_HOTLIGHT)))
				{
					//inButton = 1;
					//setEditButtonImage(NULL, lpDrawItem->hDC, IDB_ON);
				}
				else
				{
					//inButton = 0;
					//setEditButtonImage(NULL, lpDrawItem->hDC, IDB_OFF);
				}
			}

			return TRUE;
		}
	}
}