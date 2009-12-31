#pragma once
#include "HoverButton.h"

enum {	CID_MAIN_OFFSET = 0, CID_TEXT_OFFSET	= 1, CID_OK_OFFSET = 2, CID_CANCEL_OFFSET = 3 } ;

typedef struct _EditButton
{
	HoverButton *mainButton;
	HoverButton *okButton;
	HoverButton *cancelButton;
	HWND hEdit;
	HWND hParent;
	HINSTANCE hInstance;
	RECT editRect;
	int inTextMode;
	int onImage;
	int offImage;

}EditButton;

EditButton *createEditButton(HWND hWndParent, HINSTANCE hInstance, int x, int y,
							   int width, int height, int controlId, int onImage, int offImage, TCHAR *caption);
void setEditButtonText(EditButton *editButton, TCHAR *caption);
void setEditButtonStateImages(EditButton *editButton, int onImage, int offImage);
void setEditButtonFont(EditButton *editButton, TCHAR *fontName, int fontSize);
void setEditButtonTextColor(EditButton *editButton, COLORREF color);
void showEditButtonEdit(EditButton *editButton, int show);
void setDefaultEditButtonProc(WNDPROC wndProc);
int getEditButtonControlId(int cId);