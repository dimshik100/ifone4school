#include "StdAfx.h"
#include "EditButton.h"
#include "resource.h"
#include "Miscellaneous.h"

#define EDIT_BUTTON_CTL_ID	5000
#define FUNC_BUTTON_SIZE	20

WNDPROC wndDefBtnProc = NULL;
WNDPROC wndDefEditProc = NULL;

LRESULT CALLBACK	EditBtnProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		FindEditInEditButtonProc(HWND hwnd, LPARAM lParam);

EditButton *createEditButton(HWND hWndParent, HINSTANCE hInstance, int x, int y,
							   int width, int height, int controlId, int onImage, int offImage, TCHAR *caption)
{
	EditButton *newEditButton = (EditButton*)calloc(1, sizeof(EditButton));
	controlId = EDIT_BUTTON_CTL_ID + (controlId * 10);

	newEditButton->editRect.left = x + 5;
	newEditButton->editRect.right = (x + width) - 35; //15 = 5 from left, 10 from right, another 20 is for the ok/cancel buttons
	newEditButton->editRect.top = y + 5;
	newEditButton->editRect.bottom = (y + height) - 10; //10 = 5 from top and 5 from bottom
	newEditButton->onImage = onImage;
	newEditButton->offImage = offImage;

	newEditButton->hInstance = hInstance;
	newEditButton->mainButton = createHoverButton(hWndParent, hInstance, x, y, width, height, controlId, onImage, offImage, caption);
	newEditButton->okButton = createHoverButton(newEditButton->mainButton->hButton, hInstance, width - 25, 5, 19, 17,
		controlId + CID_OK_OFFSET, IDB_EDIT_BTN_OK_BTN_ON, IDB_EDIT_BTN_OK_BTN_OFF, NULL);
	newEditButton->cancelButton = createHoverButton(newEditButton->mainButton->hButton, hInstance, width - 25, 22, 19, 17,
		controlId + CID_CANCEL_OFFSET, IDB_EDIT_BTN_CANCEL_BTN_ON, IDB_EDIT_BTN_CANCEL_BTN_OFF, NULL);
	newEditButton->hEdit = CreateWindowEx(0, TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE, 5, 5, width - 35, height - 10,
		newEditButton->mainButton->hButton, (HMENU)(controlId + CID_TEXT_OFFSET), hInstance, NULL);
	showEditButtonEdit(newEditButton, FALSE);

	/// !!! Apply new window procedure to control !!! ///
	wndDefEditProc = (WNDPROC)SetWindowLong(getHoverButtonHwnd(newEditButton->mainButton), GWL_WNDPROC, (LONG_PTR)EditBtnProc);
	//wndDefBtnProc = (WNDPROC)SetWindowLong(newEditButton->hButton, GWL_WNDPROC, (LONG_PTR)EditBtnProc);

	return newEditButton;
}

void setEditButtonText(EditButton *editButton, TCHAR *caption)
{
	setHoverButtonText(editButton->mainButton, caption);
}

void setEditButtonStateImages(EditButton *editButton, int onImage, int offImage)
{
	setHoverButtonStateImages(editButton->mainButton, onImage, offImage);
}

void setEditButtonFont(EditButton *editButton, TCHAR *fontName, int fontSize)
{
	setHoverButtonFont(editButton->mainButton, fontName, fontSize);
	SendMessage(editButton->hEdit, WM_SETFONT, (WPARAM)getHoverButtonFont(editButton->mainButton), (LPARAM)TRUE);
}

HFONT getEditButtonFont(EditButton *editButton)
{
	return getHoverButtonFont(editButton->mainButton);
}

void setEditButtonTextColor(EditButton *editButton, COLORREF color)
{
	setHoverButtonTextColor(editButton->mainButton, color);
}

void showEditButtonEdit(EditButton *editButton, int show)
{
	ShowWindow(editButton->okButton->hButton, show);
	ShowWindow(editButton->cancelButton->hButton, show);
	ShowWindow(editButton->hEdit, show);
	lockHoverButtonImage(editButton->mainButton, show);
	if (show)
	{
		setHoverButtonStateImages(editButton->mainButton, editButton->onImage, editButton->onImage);
		SetFocus(editButton->hEdit);
	}
	else
		setHoverButtonStateImages(editButton->mainButton, editButton->onImage, editButton->offImage);
}

int getEditButtonControlId(int cId)
{
	if (cId < 5010)
		return cId;
	else
		return ((cId - EDIT_BUTTON_CTL_ID) / 10);
}

LRESULT CALLBACK	EditBtnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		{
			int wmId    = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			if ((wmId % 10) == CID_CANCEL_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			{
				HoverButton *hoverButton = findButton(0, hWnd);
				if (hoverButton)
					lockHoverButtonImage(hoverButton, FALSE);
				showChildWindows(hWnd, SW_HIDE);
			}
			if ((wmId % 10) == CID_OK_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			{
				HoverButton *hoverButton = findButton(0, hWnd);
				if (hoverButton)
				{
					TCHAR string[256];
					HWND hwndEdit = NULL;
					EnumChildWindows(hWnd, FindEditInEditButtonProc, (LPARAM)&hwndEdit);
					hoverButton = findButton(0, hWnd);
					if (hwndEdit && hoverButton)
					{
						GetWindowText(hwndEdit, string, 256);
						setHoverButtonText(hoverButton, string);
					}
					lockHoverButtonImage(hoverButton, FALSE);
				}
				showChildWindows(hWnd, SW_HIDE);
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			HoverButton *hoverButton = findButton(0, hWnd);
			if (hoverButton && !hoverButton->isLocked)
			{
				HWND hwndEdit = NULL;
				EnumChildWindows(hWnd, FindEditInEditButtonProc, (LPARAM)&hwndEdit);
				if (hwndEdit)
				{
					TCHAR string[256];
					getHoverButtonText(hoverButton, string, 256);
					SetWindowText(hwndEdit, string);
					SendMessage(hwndEdit, EM_SETSEL, (WPARAM)0, (LPARAM)_tcslen(string));
				}
				lockHoverButtonImage(hoverButton, TRUE);
				showChildWindows(hWnd, SW_SHOW);
			}
		}
		break;
	}

	return CallWindowProc(wndDefEditProc, hWnd, message, wParam, lParam);
}

BOOL CALLBACK FindEditInEditButtonProc(HWND hWnd, LPARAM lParam)
{
	TCHAR string[256];
	GetClassName(hWnd, string, 256);
	if (!_tcsicmp(TEXT("edit"), string))
	{
		*(HWND*)lParam = hWnd;
		return FALSE;
	}
	return TRUE;
}