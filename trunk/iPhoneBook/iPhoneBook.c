// iPhoneBook.cpp : Defines the entry point for the application.
//
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\ 
"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "stdafx.h"
#include "iPhoneBook.h"
#include "ListView.h"
#include "EditButton.h"
#include "HoverButton.h"
#include <commctrl.h>

#define MAX_LOADSTRING 100
#define TIMER_ID 10000
#define CLOCK_TIMER_ID TIMER_ID + 1
#define	PWRBTN_TIMER_ID TIMER_ID + 2

#define BUTTON_ID 20
#define BUTTON_ID_PWR (BUTTON_ID + 0)

#define BUTTON_ID_CLOCK (BUTTON_ID + 1)
#define BUTTON_ID_CONTACT (BUTTON_ID + 2)
#define BUTTON_ID_INFO (BUTTON_ID + 3)
#define BUTTON_ID_BIN (BUTTON_ID + 4)

#define BUTTON_ID_MISC1 (BUTTON_ID + 5)
#define BUTTON_ID_MISC2 (BUTTON_ID + 6)
#define BUTTON_ID_MISC3 (BUTTON_ID + 7)
#define BUTTON_ID_MISC4 (BUTTON_ID + 8)

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
VOID CALLBACK		ClockTimerProc(HWND, UINT, UINT_PTR, DWORD);
LRESULT CALLBACK	ContainerProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void createGUI(HWND hWnd, HINSTANCE hInstance);
void setImageToDC(HINSTANCE hInstance, RECT *lprc, HDC hDC, int imageId);

HoverButton 
		*hbTopBarSkype, *hbMainUnderDateBg, *hbMainCenterPic, *hbExitButton,
		*hbMainActionBtn[4], *hbMiscActionBtn[4];

HWND hwndContainerMainButtons, hwndContainerMiscButtons, hwndContainerContacts;
HWND hwndSearchBox;

HWND hLV, hList;
WNDPROC defContainerProc;
static int inButton;

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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
			//int wmEvent = HIWORD(wParam);
			switch(wmId)
			{
			case BUTTON_ID_CONTACT:
				ShowWindow(getHoverButtonHwnd(hbMainCenterPic), FALSE);
				ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), FALSE);
				ShowWindow(hwndContainerMainButtons, FALSE);
				ShowWindow(hwndContainerMiscButtons, TRUE);
				ShowWindow(hwndContainerContacts, TRUE);
				break;
			default:
				break;
			}
		}
		break;
	case WM_NOTIFY:
		{
			LRESULT ret = ListViewProc(hWnd, wParam, lParam);
			if (ret)
				return ret;
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
	int i;
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
			if (wmEvent == HOVER_BUTTON_UP)
			{
				ShowWindow(hwndContainerMiscButtons, FALSE);
				ShowWindow(hwndContainerContacts, FALSE);
				ShowWindow(getHoverButtonHwnd(hbMainCenterPic), TRUE);
				ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), TRUE);
				ShowWindow(hwndContainerMainButtons, TRUE);
				KillTimer(hWnd, PWRBTN_TIMER_ID);
			}
			else if (wmEvent == HOVER_BUTTON_DOWN)
				SetTimer(hWnd, PWRBTN_TIMER_ID, 3000, NULL);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case 3:
			if (!editBtn->inTextMode && (wmId % 10) == CID_MAIN_OFFSET)
				showEditButtonEdit(editBtn, TRUE);
			else if ((wmId % 10) == CID_OK_OFFSET)
				showEditButtonEdit(editBtn, FALSE);
			else if ((wmId % 10) == CID_CANCEL_OFFSET)
				showEditButtonEdit(editBtn, FALSE);
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
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			setImageToDC(hInst, &ps.rcPaint, hdc, IDB_IFONE_BG);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		deleteHoverButtons();
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		{
			TCHAR lpBuffer[1000], num[10];
			HoverButton *btn;

			createGUI(hWnd, hInst);
			SetTimer(hWnd, CLOCK_TIMER_ID, 500, (TIMERPROC)ClockTimerProc);
			btn = createHoverButton(hWnd, hInst, 505, 165, 178, 178, 1, IDB_ON, IDB_OFF, "Test text");
			setHoverButtonTextColor(btn, 255);
			
			setHoverButtonFont(createHoverButton(hWnd, hInst, 505+178, 155, 178, 178, 2, IDB_MAIN_WND_CLOCK_ON, IDB_MAIN_WND_CLOCK_OFF, "abcdefgh"),
				TEXT("Fixedsys Excelsior 3.01"), 24);

			setDefaultEditButtonProc(WndProc);
			editBtn = createEditButton(hWnd, hInst, 505, 165+200, 320, 44, 3, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, "Test text");
			setEditButtonFont(editBtn, TEXT("Arial"), 16);




			// Creates a list box - this keeps a list of scroll-sizes
			hList = CreateWindowEx(0, TEXT("listbox"), NULL,WS_VSCROLL |  WS_CHILD | WS_VISIBLE | LBS_HASSTRINGS | LBS_DISABLENOSCROLL , 500+460, 0, 200, 550, hWnd, NULL, hInst, NULL);
			initListViewColumns(hLV);

			for (i = 0; i < 200; i++)
			{
				// Windows API which takes a "System Error Code" (i) and converts it to an actual system error string..
				// This function is here for simple filling of the ListView
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, i, 0, lpBuffer, 1000, NULL);
				_stprintf_s(num, 10, TEXT("%03d"), i);
				// Adds an item into the list view - see ListView.cpp for details.
				addListViewItem(hLV, lpBuffer);
			}
			addListViewItem(hLV, TEXT("הצילו, המוח שלי נפטר!!"));
			addListViewItem(hLV, TEXT("A"));

			//// Define button size - not needed anymore...
			//btn1.left = 5; btn1.right = btn1.left + 178; btn1.top = 155; btn1.bottom = btn1.top + 177;
			//// Create an Owner-Drawn button and save it's handle.
			//hBtn = CreateWindowEx(0, TEXT("button"), TEXT("Test Button"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW | BS_NOTIFY, 
			//	btn1.left, btn1.top, btn1.right - btn1.left, btn1.bottom - btn1.top, hWnd, NULL, hInst, NULL);
			//// Keep a reference to the original Window Procedure of the button control.
			//defContainerProc = (WNDPROC)GetWindowLong(hBtn, GWL_WNDPROC);
			//// Set our own custom Window Procedure to handle the button events.
			//SetWindowLong(hBtn, GWL_WNDPROC, (LONG_PTR)ContainerProc);

			// Creates a standard windows type button
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

void setImageToDC(HINSTANCE hInstance, RECT *lprc, HDC hDC, int imageId)
{
	HBITMAP hbmpOld, hbmpImage;
	HDC hDCMem;

	// Create a DC in memory, compatible with the button's original DC.
	hDCMem = CreateCompatibleDC(hDC);
	// Load the selected image from the resource file.
	hbmpImage = LoadBitmap(hInstance, MAKEINTRESOURCE(imageId));
	// Select the image into the DC. Keep a reference to the old bitmap.
	hbmpOld = (HBITMAP)SelectObject(hDCMem, hbmpImage);
	// Copies the bitmap from the memory DC into the buttons DC 
	BitBlt(hDC, lprc->left, lprc->top, lprc->right, lprc->bottom, hDCMem, 0, 0, SRCCOPY);
	// Select the original memory DC's bitmap.
	SelectObject(hDCMem, hbmpOld);
	// Free resources.
	DeleteDC(hDCMem);
	DeleteObject(hbmpImage);
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
		setHoverButtonTextColor(hbMainUnderDateBg, RGB(255, 255, 255));
		setHoverButtonFont(hbMainUnderDateBg, TEXT("Arial"), 36);
		//setHoverButtonText(hbMainUnderDateBg, TEXT("Testing"));
		_tcsftime(str, 1000, TEXT("%H:%M"), &today);
		setHoverButtonText(hbMainUnderDateBg,str);
	}
}

void createGUI(HWND hWnd, HINSTANCE hInstance)
{
	HoverButton *tempBtn;
	int x, y, width, height;
	HBITMAP hBmp;

	hbTopBarSkype = createHoverButton(hWnd, hInstance, 67, 136, 320, 20, 0, IDB_TOP_BAR_SKYPE_OFF, IDB_TOP_BAR_SKYPE_OFF, NULL);
	lockHoverButtonImage(hbTopBarSkype, TRUE);
	hbMainUnderDateBg = createHoverButton(hWnd, hInstance, 67, 156, 320, 97, 0, IDB_MAIN_WND_UNDER_DATE_BG, IDB_MAIN_WND_UNDER_DATE_BG, NULL);
	lockHoverButtonImage(hbMainUnderDateBg, TRUE);
	hbMainCenterPic = createHoverButton(hWnd, hInstance, 67, 253, 320, 273, 0, IDB_MAIN_WND_CENTER_PIC, IDB_MAIN_WND_CENTER_PIC, NULL);
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
	hwndSearchBox = CreateWindowEx(0, TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE, 35, 56, 265, 23, hwndContainerContacts, NULL, hInstance, NULL);
}