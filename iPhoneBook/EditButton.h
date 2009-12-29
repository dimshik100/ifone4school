#pragma once

typedef struct _EditButton
{
	HWND hButton;
	HWND hEdit;
	HWND hOK;
	HINSTANCE hInstance;
	RECT buttonRect;
	RECT editRect;
	int inTextMode;

}EditButton;

EditButton *createEditButton(HWND hWndParent, HINSTANCE hInstance, int x, int y, int width, int height, int controlId, WNDPROC newWndProc);
void setEditButtonImage(EditButton *editButton, HDC hDC, int imageId);
void setEditButtonText(EditButton *editButton, HDC hDC, TCHAR *string);
void setEditButtonFont(EditButton *editButton, TCHAR *fontName, int fontSize);
void showEditButtonEdit(EditButton *editButton, int show);