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
	SetWindowText(editButton->hEdit, caption);
	SendMessage(editButton->hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)_tcslen(caption));
}

void setEditButtonStateImages(EditButton *editButton, int onImage, int offImage)
{
	setHoverButtonStateImages(editButton->mainButton, onImage, offImage);
}

void setEditButtonImageStretch(EditButton *editButton, int enable)
{
	setHoverButtonImageStretch(editButton->mainButton, enable);
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

void setEditButtonEditStyles(EditButton *editButton, DWORD newStyles)
{
	DWORD styles = GetWindowLong(editButton->hEdit, GWL_STYLE) & ~(ES_PASSWORD | ES_NUMBER | ES_READONLY);
	newStyles &= (ES_PASSWORD | ES_NUMBER | ES_READONLY);

	SetWindowLong(editButton->hEdit, GWL_STYLE, styles | newStyles);
}

void getEditButtonText(EditButton *editButton, TCHAR *destination, size_t length, int getUnsaved)
{
	if (editButton->inEditMode && getUnsaved)
		GetWindowText(editButton->hEdit, destination, (int)length);
	else
		getHoverButtonText(editButton->mainButton, destination, length);
}

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

void lockEditButton(EditButton *editButton, int enable)
{
	editButton->isLocked = enable;
	lockHoverButtonImage(editButton->mainButton, enable);
}

HWND getEditButtonHwnd(EditButton *editButton)
{
	return getHoverButtonHwnd(editButton->mainButton);
}

int getEditButtonControlId(int cId)
{
	return (cId < 5010) ? cId : ((cId - EDIT_BUTTON_CTL_ID) / 10);
}

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

			//HoverButton *hoverButton = findHoverButton(0, hWnd);
			//if (hoverButton && !hoverButton->isLocked)
			//{
			//	HWND hwndEdit = NULL;
			//	EnumChildWindows(hWnd, FindEditInEditButtonProc, (LPARAM)&hwndEdit);
			//	if (hwndEdit)
			//	{
			//		TCHAR string[256];
			//		getHoverButtonText(hoverButton, string, 256);
			//		SetWindowText(hwndEdit, string);
			//		SendMessage(hwndEdit, EM_SETSEL, (WPARAM)0, (LPARAM)_tcslen(string));
			//	}
			//	lockHoverButtonImage(hoverButton, TRUE);
			//	showChildWindows(hWnd, SW_SHOW);
			//}
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
				//invalidateButtonRect(editButton->mainButton);
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

void deleteEditButtons()
{
	listFree(&editButtonList);
}

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