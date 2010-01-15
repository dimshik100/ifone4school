// iPhoneBook.cpp : Defines the entry point for the application.
//
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\ 
"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "stdafx.h"
#include "iPhoneBook.h"
#include "ListView.h"
#include "EditButton.h"
#include "HoverButton.h"
#include "ScrollContainer.h"
#include "Clock.h"
#include "PhoneBook.h"
#include "Miscellaneous.h"
#include "SkypeAPI.h"
#include <commctrl.h>

#define MAX_LOADSTRING 100
#define TIMER_ID 10000
#define CLOCK_TIMER_ID TIMER_ID + 1
#define	PWRBTN_TIMER_ID TIMER_ID + 2

#define CONTROL_ID 20
#define BUTTON_ID_PWR				(CONTROL_ID + 0)
#define BUTTON_ID_CLOCK				(CONTROL_ID + 1)
#define BUTTON_ID_CONTACT			(CONTROL_ID + 2)
#define BUTTON_ID_INFO				(CONTROL_ID + 3)
#define BUTTON_ID_BIN				(CONTROL_ID + 4)
#define BUTTON_ID_MISC1				(CONTROL_ID + 5)
#define BUTTON_ID_MISC2				(CONTROL_ID + 6)
#define BUTTON_ID_MISC3				(CONTROL_ID + 7)
#define BUTTON_ID_MISC4				(CONTROL_ID + 8)
#define BUTTON_ID_YES				(CONTROL_ID + 9)
#define BUTTON_ID_NO				(CONTROL_ID + 10)
#define INFO_ID_LAST_NAME			(CONTROL_ID + 11)
#define INFO_ID_FIRST_NAME			(CONTROL_ID + 12)
#define INFO_ID_PHONE				(CONTROL_ID + 13)
#define INFO_ID_WEB					(CONTROL_ID + 14)
#define INFO_ID_EMAIL				(CONTROL_ID + 15)
#define INFO_ID_AGE					(CONTROL_ID + 16)
#define INFO_ID_ADDRESS_CNT			(CONTROL_ID + 17)
#define INFO_ID_ADDRESS_CTY			(CONTROL_ID + 18)
#define INFO_ID_ADDRESS_STR			(CONTROL_ID + 19)
#define INFO_ID_ADDRESS_NUM			(CONTROL_ID + 20)
#define INFO_ID_ADDRESS				(CONTROL_ID + 21)
#define INFO_ID_SKYPE				(CONTROL_ID + 22)
#define BUTTON_ID_ALL_CONTACTS		(CONTROL_ID + 23)
#define BUTTON_ID_EDIT_CONTACT		(CONTROL_ID + 24)
#define BUTTON_ID_SAVE_CONTACT		(CONTROL_ID + 25)
#define BUTTON_ID_CANCEL_CONTACT	(CONTROL_ID + 26)
#define LV_CONTACTS_ID				(CONTROL_ID + 30)
#define EDIT_ID_SEARCH				(CONTROL_ID + 31)

typedef enum {	SCREEN_MAIN, SCREEN_CONTACTS, SCREEN_MEM_INFO, SCREEN_TRASH, 
				SCREEN_CONTACT_INFO, SCREEN_CONTACT_EDIT, SCREEN_CLOCK, SCREEN_CALL_MODE }
ScreenMode;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

VOID CALLBACK		ClockTimerProc(HWND, UINT, UINT_PTR, DWORD);
LRESULT CALLBACK	ContainerProc(HWND, UINT, WPARAM, LPARAM);
void CALLBACK		skypeCallStatusCallback(SkypeCallObject *skypeCallObject);
void CALLBACK		skypeConnectionStatusCallback(SkypeApiInitStatus skypeApiInitStatus);

void createGUI(HWND hWnd, HINSTANCE hInstance);
void showChildContainers(int nCmdShow);
void enableChildContainers(BOOL value);


HoverButton 
		*hbTopBarSkype, *hbMainUnderDateBg, *hbMainCenterPic, *hbExitButton, *hbContactInfoAddress,
		*hbMainActionBtn[4], *hbMiscActionBtn[4], *hbYes, *hbNo, *allContactsButton, *editContactButton;

EditButton *ebContactInfo[10];
	/* 
	0- First Name
	1- Last Name
	2- Phone Number 
	3- Skype Name
	4- Email
	5- Website
	6- Address Country
	7- Address City
	8- Address Street
	9- Address Number
	*/

HWND hwndContainerMainButtons, hwndContainerMiscButtons, hwndContainerContacts, hwndContainerContactDetails,
		hwndAddressContainer;
HWND hwndSearchBox, hwndConfirmDialog;
HWND hLV;
WNDPROC defContainerProc;
const RECT ifoneScreenRect = { 67, 136, 386, 615 };
DynamicListC contactList = NULL;
ScreenMode screenMode = SCREEN_MAIN;
int isConfirmOn = FALSE;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	INITCOMMONCONTROLSEX initCommCtl;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	initCommCtl.dwICC = ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;
	initCommCtl.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&initCommCtl);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_IPHONEBOOK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IPHONEBOOK));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IPHONEBOOK));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_IPHONEBOOK);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW - WS_MAXIMIZEBOX,
      25, 25, 830, 820, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void CALLBACK skypeCallStatusCallback(SkypeCallObject *skypeCallObject)
{
	if (skypeCallObject && skypeCallObject->object == OBJECT_CALL)
	{
		TCHAR str[256] = {0};
		HDC hdc;
		int strPos = 0;
		RECT rc;

		if (skypeCallObject->type == CALLTYPE_INCOMING_P2P || skypeCallObject->type == CALLTYPE_INCOMING_PSTN)
			strPos = _stprintf_s(str, 256, TEXT("Incoming call...\n"));
		else if (skypeCallObject->type == CALLTYPE_OUTGOING_P2P || skypeCallObject->type == CALLTYPE_OUTGOING_PSTN)
			strPos = _stprintf_s(str, 256, TEXT("Outgoing call...\n"));
		strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Call ID: %d\n"), skypeCallObject->callId);
		if (skypeCallObject->partnerHandle)
			strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Partner:\n\t%s\n"), skypeCallObject->partnerHandle);
		if (skypeCallObject->partnerDisplayName)
			strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Display name:\n\t%s\n"), skypeCallObject->partnerDisplayName);
			strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Duration: %02dh:%02dm:%02ds\n"), skypeCallObject->duration / 3600, (skypeCallObject->duration % 3600) / 60, (skypeCallObject->duration % 60));
			strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Status: %d\n"), skypeCallObject->status);
			strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Property: %d\n"), skypeCallObject->property);
		switch (skypeCallObject->property)
		{
		case CALLPROPERTY_DURATION:
			strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Duration update\n"), skypeCallObject->property);
			break;
		case CALLPROPERTY_STATUS:
			strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Status notification\n"), skypeCallObject->property);
			break;
		default:
			strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Other property notification\n"), skypeCallObject->property);
			break;
		}
		strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("skypeCallObject = 0x%p"), skypeCallObject);

		hdc = GetDC(getHoverButtonHwnd(hbMainCenterPic));
		GetClientRect(getHoverButtonHwnd(hbMainCenterPic), &rc);
		FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW+1));
		DrawText(hdc, str, strPos, &rc, DT_WORDBREAK | DT_EXPANDTABS);
		ReleaseDC(getHoverButtonHwnd(hbMainCenterPic), hdc);		
	}
}

void CALLBACK skypeConnectionStatusCallback(SkypeApiInitStatus skypeApiInitStatus)
{
	switch (skypeApiInitStatus)
	{
	// User defined constants
	case ATTACH_ACTIVE:
		break;
	case ATTACH_CONNECTION_LOST:
		setHoverButtonStateImages(hbTopBarSkype, IDB_TOP_BAR_SKYPE_OFF, IDB_TOP_BAR_SKYPE_OFF);
		break;
	// Skype defined constants
	case ATTACH_AVAILABLE:
		break;
	case ATTACH_NOT_AVAILABLE:
		setHoverButtonStateImages(hbTopBarSkype, IDB_TOP_BAR_SKYPE_OFF, IDB_TOP_BAR_SKYPE_OFF);
		break;
	case ATTACH_PENDING:
		break;
	case ATTACH_REFUSED:
		setHoverButtonStateImages(hbTopBarSkype, IDB_TOP_BAR_SKYPE_OFF, IDB_TOP_BAR_SKYPE_OFF);
		break;
	case ATTACH_SUCCESS:
		setHoverButtonStateImages(hbTopBarSkype, IDB_TOP_BAR_SKYPE_ON, IDB_TOP_BAR_SKYPE_ON);
		break;
	}
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

LRESULT CALLBACK ContainerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		{
			int wmId    = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			switch(getEditButtonControlId(wmId))
			{
			case BUTTON_ID_CLOCK:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					if (isOsVista())
					{
					// If we support transparent confirm dialog, we must set it's position
					// to be centered on the ifonebook screen
						POINT pt = {0, 0};
						ClientToScreen(GetParent(hWnd), &pt);
						MoveWindow(hwndConfirmDialog, 
							pt.x + ifoneScreenRect.left + (ifoneScreenRect.right - ifoneScreenRect.left - 277) / 2,
							pt.y + ifoneScreenRect.top + (ifoneScreenRect.bottom - ifoneScreenRect.top - 103) / 2,
							277, 103, TRUE);
					}
					ShowWindow(hwndConfirmDialog, SW_SHOW);
					isConfirmOn = TRUE;
					enableChildContainers(FALSE);
					screenMode = SCREEN_CLOCK;
				}
				break;
			//case INFO_ID_CNTNAME:
			//	if ((wmId % 10) == CID_MAIN_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			//		showEditButtonEdit(ebContactInfo[0], TRUE);
			//	else if ((wmId % 10) == CID_OK_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			//		showEditButtonEdit(ebContactInfo[0], FALSE);
			//	else if ((wmId % 10) == CID_CANCEL_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			//		showEditButtonEdit(ebContactInfo[0], FALSE);
			//	break;
			//case INFO_ID_PHONE:
			//	if ((wmId % 10) == CID_MAIN_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			//		showEditButtonEdit(ebContactInfo[1], TRUE);
			//	else if ((wmId % 10) == CID_OK_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			//		showEditButtonEdit(ebContactInfo[1], FALSE);
			//	else if ((wmId % 10) == CID_CANCEL_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP)
			//		showEditButtonEdit(ebContactInfo[1], FALSE);
				break;
			case BUTTON_ID_CONTACT:
			case BUTTON_ID_ALL_CONTACTS:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					showChildContainers(SW_HIDE);
					ShowWindow(hwndContainerMiscButtons, SW_SHOW);
					ShowWindow(hwndContainerContacts, SW_SHOW);
					screenMode = SCREEN_CONTACTS;
					fillListView(hLV, getContactListInitiated(), NULL);
					SetFocus(hwndSearchBox);
					screenMode = SCREEN_CONTACTS;
				}
				break;
			case BUTTON_ID_MISC3: // Go to Single contact info screen
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					showChildContainers(SW_HIDE);
					ShowWindow(hwndContainerMiscButtons, SW_SHOW);
					ShowWindow(hwndContainerContactDetails, SW_SHOW);
					screenMode = SCREEN_CONTACT_INFO;
				}
				break;
			case BUTTON_ID_YES:
			case BUTTON_ID_NO:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					ShowWindow(hwndConfirmDialog, SW_HIDE);
					isConfirmOn = FALSE;
					enableChildContainers(TRUE);
				}
				break;
			case EDIT_ID_SEARCH:
				if (wmEvent == EN_CHANGE)
				{
					TCHAR str[256];
					GetWindowText((HWND)lParam, str, 256);
					fillListView(hLV, getContactListLocal(), str);
				}
			default:
				break;
			}
			ClockTimerProc(NULL, 0, 0, 0);
		}
		break;
	case WM_NOTIFY:
		{
			LRESULT ret = ListViewProc(hWnd, wParam, lParam);
			if (ret)
				return ret;
			//if (lParam && ((LPNMHDR)lParam)->hwndFrom == hLV && ((LPNMHDR)lParam)->code == LVN_ITEMACTIVATE)
			//{
			//	LVITEM lvItem;
			//	int index = ((LPNMITEMACTIVATE)lParam)->iItem;

			//	if (index >= 0)
			//	{
			//		lvItem.iItem = index;
			//		lvItem.mask = LVIF_PARAM;					
			//		ListView_GetItem(hLV, &lvItem);
			//	}
			//	else
			//		lvItem.lParam = NULL;
			//	if (lvItem.lParam)
			//	{
			//		lockHoverButtonImage(hbMiscActionBtn[1], FALSE);
			//		lockHoverButtonImage(hbMiscActionBtn[2], FALSE);
			//	}
			//	else
			//	{
			//		lockHoverButtonImage(hbMiscActionBtn[1], TRUE);
			//		lockHoverButtonImage(hbMiscActionBtn[2], TRUE);
			//	}
			//	return FALSE;
			//}
		}
		break;
	case WM_DESTROY:
		{
			// If we had set a background image to our container, we should free it's memory/resources.
			HBITMAP hBmp;
			hBmp = (HBITMAP)SendMessage(hWnd, STM_GETIMAGE, IMAGE_BITMAP, (LPARAM)0);
			if (hBmp)
				DeleteObject(hBmp);
		}
		break;
	default:
		break;
	}

	return CallWindowProc(defContainerProc, hWnd, message, wParam, lParam);
}

EditButton *editBtn;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	static int scrolling;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (getEditButtonControlId(wmId))
		{
		case BUTTON_ID_PWR:
			if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
			{
				if (screenMode != SCREEN_MAIN)
				{
					showChildContainers(SW_HIDE);
					ShowWindow(getHoverButtonHwnd(hbMainCenterPic), SW_SHOW);
					ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), SW_SHOW);
					ShowWindow(hwndContainerMainButtons, SW_SHOW);
				}
				KillTimer(hWnd, PWRBTN_TIMER_ID);
				screenMode = SCREEN_MAIN;
			}
			else if (wmEvent == HOVER_BUTTON_MOUSE_DOWN_LEAVE)
				KillTimer(hWnd, PWRBTN_TIMER_ID);
			else if (wmEvent == HOVER_BUTTON_LMOUSE_DOWN)
				SetTimer(hWnd, PWRBTN_TIMER_ID, 3000, NULL);
			ClockTimerProc(NULL, 0, 0, 0);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case 3:
			if ((wmId % 10) == CID_MAIN_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP);
			else if ((wmId % 10) == CID_OK_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP);
			else if ((wmId % 10) == CID_CANCEL_OFFSET && wmEvent == HOVER_BUTTON_LMOUSE_UP);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_TIMER:
		if (wParam == PWRBTN_TIMER_ID)
		{
			DestroyWindow(hWnd);
			return FALSE;
		}
		break;
	case WM_NCHITTEST:
		{
			LRESULT uHitTest;
			// Gets the part of the window the mouse pointer is now over
			uHitTest = DefWindowProc(hWnd, WM_NCHITTEST, wParam, lParam);
			// If we're over the client area, return TitleBar area (caption area) 
			// so that we can move the window by dragging on the client area
			// Else return whatever the part we're on.
			if(uHitTest == HTCLIENT && !isConfirmOn)
				return HTCAPTION;
			else
				return uHitTest;
		}
		break;
	case WM_ERASEBKGND:
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			setImageToDc(hInst, &rc, &rc, (HDC)wParam, IDB_IFONE_BG);
		}
		return TRUE;
		break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		deleteHoverButtons();
		disconnectSkype(hInst);
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		{
			HoverButton *btn;

			createGUI(hWnd, hInst);
			setSkypeCallStatusCallback(skypeCallStatusCallback);
			setSkypeConnectionStatusCallback(skypeConnectionStatusCallback);
			connectSkype(hInst);
			SetTimer(hWnd, CLOCK_TIMER_ID, 500, (TIMERPROC)ClockTimerProc);
			btn = createHoverButton(hWnd, hInst, 450, 500, 178, 178, 1, IDB_ON, IDB_OFF, TEXT("Test text"));
			setHoverButtonTextColor(btn, 255);
			
			setHoverButtonFont(createHoverButton(hWnd, hInst, 450+178, 500, 178, 178, 2, IDB_MAIN_WND_CLOCK_ON, IDB_MAIN_WND_CLOCK_OFF, TEXT("abcdefgh")),
				TEXT("Fixedsys Excelsior 3.01"), 24);

			editBtn = createEditButton(hWnd, hInst, 450, 500+200, 320, 44, 3, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("Test text"));
			setEditButtonFont(editBtn, TEXT("Arial"), 16);

			initListViewColumns(hLV);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

VOID CALLBACK		ClockTimerProc(HWND hWnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
	TCHAR str[1000];
	time_t ltime;
	struct tm today;
	UNREFERENCED_PARAMETER(hWnd), UNREFERENCED_PARAMETER(message), UNREFERENCED_PARAMETER(idEvent), UNREFERENCED_PARAMETER(dwTime);

	time(&ltime);
	if (localtime_s(&today, &ltime) == ERROR_SUCCESS)
	{
		_tcsftime(str, 1000, TEXT("%H:%M"), &today);
		if (screenMode == SCREEN_MAIN)
		{
			setHoverButtonText(hbMainUnderDateBg, str);
			if (getHoverButtonText(hbTopBarSkype, str, 1000) > 0)
				setHoverButtonText(hbTopBarSkype, NULL);
		}
		else
			setHoverButtonText(hbTopBarSkype, str);
	}
}

void showChildContainers(int nCmdShow)
{	
	ShowWindow(getHoverButtonHwnd(hbMainCenterPic), nCmdShow);
	ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), nCmdShow);
	ShowWindow(hwndContainerMainButtons, nCmdShow);
	ShowWindow(hwndContainerMiscButtons, nCmdShow);
	ShowWindow(hwndContainerContacts, nCmdShow);
	ShowWindow(hwndContainerContactDetails, nCmdShow);
}

void enableChildContainers(BOOL value)
{	
	EnableWindow(getHoverButtonHwnd(hbMainCenterPic), value);
	EnableWindow(getHoverButtonHwnd(hbMainUnderDateBg), value);
	EnableWindow(hwndContainerMainButtons, value);
	EnableWindow(hwndContainerMiscButtons, value);
	EnableWindow(hwndContainerContacts, value);
	InvalidateRect(GetParent(hwndConfirmDialog), NULL, FALSE);
}

void createGUI(HWND hWnd, HINSTANCE hInstance)
{
	HoverButton *tempBtn;

	int x, y, width, height;
	HBITMAP hBmp;

	screenMode = SCREEN_MAIN;
	hbTopBarSkype = createHoverButton(hWnd, hInstance, 67, 136, 320, 20, 0, IDB_TOP_BAR_SKYPE_OFF, IDB_TOP_BAR_SKYPE_OFF, NULL);
	lockHoverButtonImage(hbTopBarSkype, TRUE);
	setHoverButtonTextColor(hbTopBarSkype, RGB(0, 0, 0));
	hbMainUnderDateBg = createHoverButton(hWnd, hInstance, 67, 156, 320, 97, 0, IDB_MAIN_WND_UNDER_DATE_BG, IDB_MAIN_WND_UNDER_DATE_BG, NULL);
	lockHoverButtonImage(hbMainUnderDateBg, TRUE);
	setHoverButtonTextColor(hbMainUnderDateBg, RGB(255, 255, 255));
	setHoverButtonFont(hbMainUnderDateBg, TEXT("Arial"), 36);
	ClockTimerProc(NULL, 0, 0, 0); // Draw clock.
	hbMainCenterPic = createHoverButton(hWnd, hInstance, 67, 253, 320, 271, 0, IDB_MAIN_WND_CENTER_PIC, IDB_MAIN_WND_CENTER_PIC, NULL);
	lockHoverButtonImage(hbMainCenterPic, TRUE);
	hbExitButton = createHoverButton(hWnd, hInstance, 193, 634, 69, 69, BUTTON_ID_PWR, IDB_EXIT_BUTTON_ON, IDB_EXIT_BUTTON_OFF, NULL);
	setHoverButtonAsPushButton(hbExitButton, TRUE);
	hwndContainerMainButtons = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD | WS_VISIBLE, 67, 524, 320, 92, hWnd, NULL, hInstance, NULL);
	defContainerProc = (WNDPROC)SetWindowLong(hwndContainerMainButtons, GWL_WNDPROC, (LONG_PTR)ContainerProc);

	tempBtn = createHoverButton(hwndContainerMainButtons, hInstance, 0, 0, 10, 92, 0, IDB_MAIN_WND_LOW_BAR_LEFT, IDB_MAIN_WND_LOW_BAR_LEFT, NULL);
	lockHoverButtonImage(tempBtn, TRUE);
	tempBtn = createHoverButton(hwndContainerMainButtons, hInstance, 311, 0, 9, 92, 0, IDB_MAIN_WND_LOW_BAR_RIGHT, IDB_MAIN_WND_LOW_BAR_RIGHT, NULL);
	lockHoverButtonImage(tempBtn, TRUE);
	x = 10;
	y = 0;
	width = 75;
	height = 92;

	hbMainActionBtn[0] = createHoverButton(hwndContainerMainButtons, hInstance, x, y, width, height, BUTTON_ID_CLOCK, IDB_MAIN_WND_CLOCK_ON, IDB_MAIN_WND_CLOCK_OFF, NULL);
	x += width;
	hbMainActionBtn[1] = createHoverButton(hwndContainerMainButtons, hInstance, x, y, width, height, BUTTON_ID_CONTACT, IDB_MAIN_WND_CONTACT_ON, IDB_MAIN_WND_CONTACT_OFF, NULL);
	x += width;
	hbMainActionBtn[2] = createHoverButton(hwndContainerMainButtons, hInstance, x, y, width+1, height, BUTTON_ID_INFO, IDB_MAIN_WND_INFO_ON, IDB_MAIN_WND_INFO_OFF, NULL);
	x += width+1;
	hbMainActionBtn[3] = createHoverButton(hwndContainerMainButtons, hInstance, x, y, width, height, BUTTON_ID_BIN, IDB_MAIN_WND_BIN_EMPTY_ON, IDB_MAIN_WND_BIN_EMPTY_OFF, NULL);
	
	hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CONTACT_WND_BUTTON_BG));
	hwndContainerMiscButtons = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD/* | WS_VISIBLE*/ | SS_BITMAP, 67, 550, 320, 66, hWnd, NULL, hInstance, NULL);
	SendMessage(hwndContainerMiscButtons, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
	SetWindowLong(hwndContainerMiscButtons, GWL_WNDPROC, (LONG_PTR)ContainerProc);
	x = 5;
	y = 0;
	width = 70;
	height = 66;

	hbMiscActionBtn[0] = createHoverButton(hwndContainerMiscButtons, hInstance, x, y, width, height, BUTTON_ID_MISC1, IDB_CONTACT_WND_BUTTON_CALL_ON, IDB_CONTACT_WND_BUTTON_CALL_OFF, NULL);
	x += width+10;
	hbMiscActionBtn[1] = createHoverButton(hwndContainerMiscButtons, hInstance, x, y, width, height, BUTTON_ID_MISC2, IDB_CONTACT_WND_BUTTON_INFO_ON, IDB_CONTACT_WND_BUTTON_INFO_OFF, NULL);
	x += width+10;
	hbMiscActionBtn[2] = createHoverButton(hwndContainerMiscButtons, hInstance, x, y, width, height, BUTTON_ID_MISC3, IDB_CONTACT_WND_BUTTON_EDIT_ON, IDB_CONTACT_WND_BUTTON_EDIT_OFF, NULL);
	x += width+10;
	hbMiscActionBtn[3] = createHoverButton(hwndContainerMiscButtons, hInstance, x, y, width, height, BUTTON_ID_MISC4, IDB_CONTACT_WND_BUTTON_DEL_ON, IDB_CONTACT_WND_BUTTON_DEL_OFF, NULL);

	hwndContainerContacts = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD/* | WS_VISIBLE*/, 67, 156, 320, 394, hWnd, NULL, hInstance, NULL);
	SetWindowLong(hwndContainerContacts, GWL_WNDPROC, (LONG_PTR)ContainerProc);
	// Create an Custom-Drawn list view control - see ListView.cpp for details.
	hLV = createListView(hwndContainerContacts, hInst, 0, 88, 320, 306);
	tempBtn = createHoverButton(hwndContainerContacts, hInstance, 0, 0, 320, 44, 0, IDB_CONTACT_WND_APP_NAME, IDB_CONTACT_WND_APP_NAME, NULL);
	lockHoverButtonImage(tempBtn, TRUE);
	tempBtn = createHoverButton(hwndContainerContacts, hInstance, 0, 44, 320, 44, 0, IDB_CONTACT_WND_SEARCH, IDB_CONTACT_WND_SEARCH, NULL);
	lockHoverButtonImage(tempBtn, TRUE);
	EnableWindow(tempBtn->hButton, FALSE);
	hwndSearchBox = CreateWindowEx(0, TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE, 35, 56, 265, 23, hwndContainerContacts, (HMENU)EDIT_ID_SEARCH, hInstance, NULL);
	
	// confirm dialog
	hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ALERT_BG));
	if (isOsVista())
	{
		POINT pt = {0, 0};
		ClientToScreen(hWnd, &pt);
		hwndConfirmDialog = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST, TEXT("static"), NULL,  SS_BITMAP | WS_POPUP,
			pt.x + ifoneScreenRect.left + (ifoneScreenRect.right - ifoneScreenRect.left - 277) / 2,
			pt.y + ifoneScreenRect.top + (ifoneScreenRect.bottom - ifoneScreenRect.top - 103) / 2,
			277, 103, hWnd, NULL, hInstance, NULL);
		SetLayeredWindowAttributes(hwndConfirmDialog, 0, 192, LWA_ALPHA);
	}
	else
	{
		hwndConfirmDialog = CreateWindowEx(0, TEXT("static"), NULL,  WS_CHILD | SS_BITMAP,
			ifoneScreenRect.left + (ifoneScreenRect.right - ifoneScreenRect.left - 277) / 2,
			ifoneScreenRect.top + (ifoneScreenRect.bottom - ifoneScreenRect.top - 103) / 2,
			277, 103, hWnd, NULL, hInstance, NULL);
	}
	SetWindowLong(hwndConfirmDialog, GWL_WNDPROC, (LONG_PTR)ContainerProc);
	SendMessage(hwndConfirmDialog, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
	hbYes = createHoverButton(hwndConfirmDialog, hInstance, 8, 53, 128, 43, BUTTON_ID_YES, IDB_ALERT_YES_ON, IDB_ALERT_YES_OFF, NULL);
	hbNo = createHoverButton(hwndConfirmDialog, hInstance, 143, 53, 128, 43, BUTTON_ID_NO, IDB_ALERT_NO_ON, IDB_ALERT_NO_OFF, NULL);


	//createClock(hWnd, hInstance, 450, 0, 320, 480, 0, IDB_CLOCK_WND_BG);


	// create contact details 
	//http://www.codeproject.com/KB/dialog/scroll_dialog.aspx
	hwndContainerContactDetails = createScrollContainer(hWnd, hInstance, 0, 67, 156, 320, 394, 320 - GetSystemMetrics(SM_CXVSCROLL), 900, 0, IDB_CONTACT_INFO_WND_BG);
	//SetWindowLong(hwndContainerContactDetails, GWL_WNDPROC, (LONG_PTR)ContainerProc);
	allContactsButton = createHoverButton(hwndContainerContactDetails, hInstance, 13, 7, 91, 30, BUTTON_ID_ALL_CONTACTS, IDB_CONTACT_INFO_ALL_CONTACTS, IDB_CONTACT_INFO_ALL_CONTACTS, NULL);
	editContactButton = createHoverButton(hwndContainerContactDetails, hInstance, 254, 7, 50, 30, BUTTON_ID_EDIT_CONTACT, IDB_CONTACT_INFO_EDIT_CONTACT, IDB_CONTACT_INFO_EDIT_CONTACT, NULL);

	x = 0;
	y = 44;
	width = 320 - GetSystemMetrics(SM_CXVSCROLL);
	height = 44;

	ebContactInfo[0] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_LAST_NAME, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("Moshe"));
	y += height+15;
	ebContactInfo[1] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_FIRST_NAME, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("Cohen"));
	y += height+15;
	ebContactInfo[2] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_PHONE, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("98765432"));
	y += height+15;
	ebContactInfo[3] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_SKYPE, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("golom"));
	y += height+15;
	ebContactInfo[4] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_EMAIL, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("a@b.c"));
	y += height+15;
	ebContactInfo[5] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_WEB, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("www.asdfasdf.com"));
	y += height+15;
	ebContactInfo[6] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_AGE, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("34"));
	y += height+15;
	ebContactInfo[7] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_ADDRESS_CNT, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("Israel"));
	y += height+15;
	ebContactInfo[8] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_ADDRESS_CTY, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("Holon"));
	y += height+15;
	ebContactInfo[9] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_ADDRESS_STR, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("Golomb"));
	y += height+15;
	ebContactInfo[10] = createEditButton(hwndContainerContactDetails, hInstance, x, y, width, height, INFO_ID_ADDRESS_NUM, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, TEXT("52"));
	y += height+15;
	for (y = 0; y < 11; y++)
		setEditButtonImageStretch(ebContactInfo[y], TRUE);

	//setEditButtonFont(ebContactInfo[0], TEXT("Arial"), 10);	
	//hwndAddressContainer = CreateWindowEx(WS_EX_TOPMOST, TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP, 0, 44, 320, 264, hwndContainerContactDetails, NULL, hInstance, NULL);
}