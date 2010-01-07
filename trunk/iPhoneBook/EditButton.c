#include "StdAfx.h"
#include "EditButton.h"
#include "resource.h"

#define EDIT_BUTTON_CTL_ID	5000
#define FUNC_BUTTON_SIZE	20

WNDPROC wndDefEditBtnProc = NULL;
WNDPROC wndDefBtnProc = NULL;
WNDPROC wndDefEditProc = NULL;

LRESULT CALLBACK	EditBtnProc(HWND, UINT, WPARAM, LPARAM);

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
	newEditButton->okButton = createHoverButton(newEditButton->mainButton->hButton, hInstance, width - 25, 5, 20, 20,
		controlId + CID_OK_OFFSET, IDB_CONTACT_WND_BUTTON_CALL_ON, IDB_CONTACT_WND_BUTTON_CALL_OFF, NULL);
	newEditButton->cancelButton = createHoverButton(newEditButton->mainButton->hButton, hInstance, width - 25, height - 25, 20, 20,
		controlId + CID_CANCEL_OFFSET, IDB_CONTACT_WND_BUTTON_INFO_ON, IDB_CONTACT_WND_BUTTON_INFO_OFF, NULL);
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
}

void setEditButtonTextColor(EditButton *editButton, COLORREF color)
{
	setHoverButtonTextColor(editButton->mainButton, color);
}

void setDefaultEditButtonProc(WNDPROC wndProc)
{
	wndDefEditBtnProc = wndProc;
}

void showEditButtonEdit(EditButton *editButton, int show)
{
	ShowWindow(editButton->okButton->hButton, show);
	ShowWindow(editButton->cancelButton->hButton, show);
	ShowWindow(editButton->hEdit, show);
	lockHoverButtonImage(editButton->mainButton, show);
	editButton->inTextMode = show;
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
		if (wndDefEditBtnProc)
			CallWindowProc(wndDefEditBtnProc, hWnd, message, wParam, lParam);
		break;
	}

	return CallWindowProc(wndDefEditProc, hWnd, message, wParam, lParam);
}