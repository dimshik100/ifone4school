#pragma once

typedef struct _EditButton
{
	HWND hButton;
	HWND hEdit;
	HWND hOK;
	HWND hCancel;
	HINSTANCE hInstance;
	RECT buttonRect;
	RECT editRect;
	int inTextMode;
	int onImage;
	int offImage;

}EditButton;

EditButton *createEditButton(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height, int controlId);
void setEditButtonImage(EditButton *editButton, HDC hDC, int imageId);
void setEditButtonText(EditButton *editButton, HDC hDC, TCHAR *string);
void setEditButtonFont(EditButton *editButton, TCHAR *fontName, int fontSize);
void showEditButtonEdit(EditButton *editButton, int show);
void setDefaultEditButtonProc(WNDPROC wndProc);