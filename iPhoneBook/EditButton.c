#include "StdAfx.h"
#include "EditButton.h"
#include "resource.h"
#include "Miscellaneous.h"
#include "DynamicListC.h"

#define EDIT_BUTTON_CTL_ID	5000
#define FUNC_BUTTON_SIZE	20

WNDPROC wndDefBtnProc = NULL;
WNDPROC wndDefEditProc = NULL;

LRESULT CALLBACK	EditBtnProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	EditCtlProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		FindEditInEditButtonProc(HWND hwnd, LPARAM lParam);

DynamicListC editButtonList = NULL;

EditButton *createEditButton(HWND hWndParent, HINSTANCE hInstance, int x, int y,
							   int width, int height, int controlId, int onImage, int offImage, TCHAR *caption)
{
	EditButton *newEditButton = (EditButton*)calloc(1, sizeof(EditButton));
	controlId = EDIT_BUTTON_CTL_ID + (controlId * 10);

	if (!editButtonList)
		listInit(&editButtonList);

	listInsertAfterEnd(editButtonList, &newEditButton);

	newEditButton->editRect.left = 5;
	newEditButton->editRect.right = width - 35; //15 = 5 from left, 10 from right, another 20 is for the ok/cancel buttons
	newEditButton->editRect.top = 5;
	newEditButton->editRect.bottom = height - 10; //10 = 5 from top and 5 from bottom
	newEditButton->onImage = onImage;
	newEditButton->offImage = offImage;
	newEditButton->cId = getEditButtonControlId(controlId);

	// Create all the child controls inside the edit button
	newEditButton->hInstance = hInstance;
	newEditButton->mainButton = createHoverButton(hWndParent, hInstance, x, y, width, height, controlId, onImage, offImage, caption);
	SetWindowLong(getHoverButtonHwnd(newEditButton->mainButton), GWL_STYLE, GetWindowLong(getHoverButtonHwnd(newEditButton->mainButton), GWL_STYLE) | WS_TABSTOP);
	newEditButton->okButton = createHoverButton(newEditButton->mainButton->hButton, hInstance, width - 25, 5, 19, 17,
		controlId + CID_OK_OFFSET, IDB_EDIT_BTN_OK_BTN_ON, IDB_EDIT_BTN_OK_BTN_OFF, NULL);
	newEditButton->cancelButton = createHoverButton(newEditButton->mainButton->hButton, hInstance, width - 25, 22, 19, 17,
		controlId + CID_CANCEL_OFFSET, IDB_EDIT_BTN_CANCEL_BTN_ON, IDB_EDIT_BTN_CANCEL_BTN_OFF, NULL);
	newEditButton->hEdit = CreateWindowEx(0, TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE, 5, 5, width - 35, height - 10,
		newEditButton->mainButton->hButton, (HMENU)(controlId + CID_TEXT_OFFSET), hInstance, NULL);
	showEditButtonEdit(newEditButton, FALSE);

	/// !!! Apply new window procedures to controls !!! ///
	wndDefBtnProc = (WNDPROC)SetWindowLong(getHoverButtonHwnd(newEditButton->mainButton), GWL_WNDPROC, (LONG_PTR)EditBtnProc);
	wndDefEditProc = (WNDPROC)SetWindowLong(newEditButton->hEdit, GWL_WNDPROC, (LONG_PTR)EditCtlProc);

	return newEditButton;
}

// Sets the text into the control
void setEditButtonText(EditButton *editButton, TCHAR *caption)
{
	setHoverButtonText(editButton->mainButton, caption);
	SetWindowText(editButton->hEdit, caption);
	SendMessage(editButton->hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)_tcslen(caption));
}

// Sets the active and inactive image
void setEditButtonStateImages(EditButton *editButton, int onImage, int offImage)
{
	setHoverButtonStateImages(editButton->mainButton, onImage, offImage);
}

// Enables or disables stretching the image into the control's Rect
void setEditButtonImageStretch(EditButton *editButton, int enable)
{
	setHoverButtonImageStretch(editButton->mainButton, enable);
}

// Sets a new font to the control
void setEditButtonFont(EditButton *editButton, TCHAR *fontName, int fontSize, int isBold)
{
	setHoverButtonFont(editButton->mainButton, fontName, fontSize, isBold);
	SendMessage(editButton->hEdit, WM_SETFONT, (WPARAM)getHoverButtonFont(editButton->mainButton), (LPARAM)TRUE);
}

// Gets the font in the current control
HFONT getEditButtonFont(EditButton *editButton)
{
	return getHoverButtonFont(editButton->mainButton);
}

// Sets the text color of the control (in "Locked" mode)
void setEditButtonTextColor(EditButton *editButton, COLORREF color)
{
	setHoverButtonTextColor(editButton->mainButton, color);
}

// Allows to set the textbox as Password, Numeric and ReadOnly
void setEditButtonEditStyles(EditButton *editButton, DWORD newStyles)
{
	DWORD styles = GetWindowLong(editButton->hEdit, GWL_STYLE) & ~(ES_PASSWORD | ES_NUMBER | ES_READONLY);
	newStyles &= (ES_PASSWORD | ES_NUMBER | ES_READONLY);

	SetWindowLong(editButton->hEdit, GWL_STYLE, styles | newStyles);
}

// Gets the text string from the control
int getEditButtonText(EditButton *editButton, TCHAR *destination, size_t length, int getUnsaved)
{
	int ret = -1;
	if (editButton->inEditMode)
	{
		if (getUnsaved)
			ret = GetWindowText(editButton->hEdit, destination, (int)length);
	}
	else
	{
		getHoverButtonText(editButton->mainButton, destination, length);
		ret = (int)_tcslen(destination);
	}
	return ret;
}

// Enables or disables the edit mode
void showEditButtonEdit(EditButton *editButton, int show)
{
	editButton->inEditMode = show;
	lockHoverButtonImage(editButton->mainButton, show);
	editButton->isLocked = show;
	show = (show) ? (SW_SHOW) : (SW_HIDE);
	ShowWindow(editButton->okButton->hButton, show);
	ShowWindow(editButton->cancelButton->hButton, show);
	ShowWindow(editButton->hEdit, show);
	if (show == SW_SHOW)
	{
		TCHAR string[256];
		setHoverButtonStateImages(editButton->mainButton, editButton->onImage, editButton->onImage);
		getHoverButtonText(editButton->mainButton, string, 256);
		SetWindowText(editButton->hEdit, string);
		SendMessage(editButton->hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)_tcslen(string));
		SetFocus(editButton->hEdit);
	}
	else
		setHoverButtonStateImages(editButton->mainButton, editButton->onImage, editButton->offImage);
}

// Locks the control to only display the "off image" on the background
void lockEditButton(EditButton *editButton, int enable)
{
	editButton->isLocked = enable;
	lockHoverButtonImage(editButton->mainButton, enable);
}

// Gets the HWND of the parent control
HWND getEditButtonHwnd(EditButton *editButton)
{
	return getHoverButtonHwnd(editButton->mainButton);
}

// Gets the control's identifier
int getEditButtonControlId(int cId)
{
	return (cId < 5010) ? cId : ((cId - EDIT_BUTTON_CTL_ID) / 10);
}

// The sub-classing procedure to handle messages of the edit control
LRESULT CALLBACK	EditCtlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		{
			EditButton *editButton;
			// If "Enter" key was pressed, send message to Edit Button to simulate OK button press
			if (wParam == VK_RETURN)
			{
				editButton = findEditButton(0, GetParent(hWnd));
				PostMessage(GetParent(hWnd), WM_COMMAND, 
					MAKELONG(EDIT_BUTTON_CTL_ID + (editButton->cId * 10) + CID_OK_OFFSET,	HOVER_BUTTON_LMOUSE_UP), 0);
				// Set focus to the next item
				SetFocus(GetNextDlgTabItem(GetParent(GetParent(hWnd)), GetParent(hWnd), FALSE));
				return FALSE;
			}
			// If "Esc" key was pressed, send message to Edit Button to simulate Cancel button press
			else if (wParam == VK_ESCAPE)
			{
				editButton = findEditButton(0, GetParent(hWnd));
				PostMessage(GetParent(hWnd), WM_COMMAND, 
					MAKELONG(EDIT_BUTTON_CTL_ID + (editButton->cId * 10) + CID_CANCEL_OFFSET,	HOVER_BUTTON_LMOUSE_UP), 0);
				return FALSE;
			}
			// If "Tab" button was pressed, search for the next WS_TABSTOP enabled window
			else if (wParam == VK_TAB)
			{
				// Set focus to the previous item
				if (GetKeyState(VK_LSHIFT) & 0xFFFF0000 || GetKeyState(VK_RSHIFT) & 0xFFFF0000)
					SetFocus(GetNextDlgTabItem(GetParent(GetParent(hWnd)), GetParent(hWnd), TRUE));
				// Set focus to the next item
				else
					SetFocus(GetNextDlgTabItem(GetParent(GetParent(hWnd)), GetParent(hWnd), FALSE));
			}
		}
	}
	return CallWindowProc(wndDefEditProc, hWnd, message, wParam, lParam);
}

// The sub-classing procedure to handle messages of the HoverButton control (the parent)
LRESULT CALLBACK	EditBtnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	EditButton *editButton;

	switch (message)
	{
	case WM_COMMAND:
		{
			int wmId    = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			if ((wmId % 10) == CID_CANCEL_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			{
				editButton = findEditButton(0, hWnd);
				showEditButtonEdit(editButton, FALSE);
			}
			if ((wmId % 10) == CID_OK_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			{
				TCHAR string[256];
				editButton = findEditButton(0, hWnd);
				GetWindowText(editButton->hEdit, string, 256);
				setHoverButtonText(editButton->mainButton, string);
				showEditButtonEdit(editButton, FALSE);
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			editButton = findEditButton(0, hWnd);
			if (!editButton->isLocked)
				showEditButtonEdit(editButton, TRUE);
		}
		break;
	case WM_PAINT:
		invalidateChildWindows(hWnd, TRUE);
		break;
	case WM_SIZE:
		{
			editButton = findEditButton(0, hWnd);
			if (editButton)
			{
				SetWindowPos(editButton->hEdit, NULL, 0, 0, LOWORD(lParam) - 35, HIWORD(lParam) - 10, SWP_NOMOVE);
				editButton->editRect.bottom = HIWORD(lParam) - 5;
				editButton->editRect.right = LOWORD(lParam) - 30;
				SetWindowPos(getHoverButtonHwnd(editButton->okButton), NULL, LOWORD(lParam) - 25, 5, 0, 0, SWP_NOSIZE);
				SetWindowPos(getHoverButtonHwnd(editButton->cancelButton), NULL, LOWORD(lParam) - 25, 22, 0, 0, SWP_NOSIZE);
			}
		}
		break;
	case WM_KEYDOWN:
		{
			// If Tab key is pressed, then search for next tab control.
			if (wParam == VK_TAB)
			{
				if (GetKeyState(VK_LSHIFT) & 0xFFFF0000 || GetKeyState(VK_RSHIFT) & 0xFFFF0000)
					SetFocus(GetNextDlgTabItem(GetParent(hWnd), hWnd, TRUE));
				else
					SetFocus(GetNextDlgTabItem(GetParent(hWnd), hWnd, FALSE));
				return FALSE;
			}
			// If "Enter" is pressed, go into edit mode.
			else if (wParam == VK_RETURN)
			{
				SendMessage(hWnd, WM_LBUTTONUP, 0, 0);
				return FALSE;
			}
		}
	// If we have a request to get focus (from Tabbing) then accept it.
	case WM_GETDLGCODE:
		return DLGC_WANTTAB;
	// If we receive focus while in Edit Mode, give the focus to the edit control.
	case WM_SETFOCUS:
		editButton = findEditButton(0, hWnd);
		if (editButton->inEditMode)
			SetFocus(editButton->hEdit);
		break;
	}

	return CallWindowProc(wndDefBtnProc, hWnd, message, wParam, lParam);
}

// Retrieves the HWND of the edit control inside the EditButton
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

// Frees the list of EditButton objects
void deleteEditButtons()
{
	listFree(&editButtonList);
}

// Retrieves a pointer to the control from the list of existing controls
EditButton *findEditButton(int cId, HWND hWnd)
{
	EditButton *editButton = NULL;

	if (!editButtonList)
		return editButton;

	for (listSelectFirst(editButtonList); listSelectCurrent(editButtonList); listSelectNext(editButtonList, NULL))
	{
		listGetValue(editButtonList, NULL, &editButton);
		if (hWnd && getHoverButtonHwnd(editButton->mainButton) == hWnd)
			return editButton;
		else if (cId && editButton->cId == cId)
			return editButton;
	}

	return editButton;
}