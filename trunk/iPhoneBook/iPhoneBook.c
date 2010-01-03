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

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	BtnProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void CreateGUI(HWND hWnd, HINSTANCE hInstance);

HoverButton 
		*hbTopBarSkype, *hbMainUnderDateBg, *hbMainCenterPic, *hbExitButton,
		*hbMainActionBtn[4], *hbMiscActionBtn[5];

HWND hwndContainerMain, hwndContainerMisc;

HWND hLV, hList, hBtn;
RECT btn1;
WNDPROC defBtnProc;
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

LRESULT CALLBACK BtnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		{
			// If we're not currently inside the button, redraw it as "Active"
			if (!inButton)
			{
				DRAWITEMSTRUCT drawItem = {0};
				inButton = 1;
				drawItem.CtlType = ODT_BUTTON;
				drawItem.CtlID = 0;
				drawItem.itemID = 0;
				drawItem.itemAction = ODA_FOCUS;
				drawItem.hwndItem = hWnd;
				// Get's parent's DC, don't forget to free resources later.
				drawItem.hDC = GetDC(hWnd);
				GetClientRect(hBtn, &drawItem.rcItem);
				drawItem.itemState = ODS_HOTLIGHT;
				// Sends a message to the button's parent window (the iPhoneBook program's
				// window) and it processes it as defined in WndProc.
				SendMessage(GetParent(hWnd), WM_DRAWITEM, 0, (LPARAM)(LPDRAWITEMSTRUCT)&drawItem);
				// !!! VERY IMPORTANT !!! Free systems resources after use.
				ReleaseDC(hBtn, drawItem.hDC);
			}
		}
		break;
	default:
		break;
	}

	return CallWindowProc(defBtnProc, hWnd, message, wParam, lParam);
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
	case WM_NOTIFY:
		if (lParam && ((LPNMHDR)lParam)->code == LVN_BEGINSCROLL)
		{
			//LPNMLVSCROLL pnmLVScroll = (LPNMLVSCROLL) lParam;
			//scrolling = 0;
	/*		dx = pnmLVScroll->dx;
			dy = pnmLVScroll->dy;*/
		}
		if (lParam && ((LPNMHDR)lParam)->code == LVN_ENDSCROLL)
		{
			LPNMLVSCROLL pnmLVScroll = (LPNMLVSCROLL) lParam;
			if (!scrolling && (pnmLVScroll->dy / 2) != 0)
			{
				TCHAR str[250];
				int scrlDelay;

				scrolling = 1;
				_stprintf_s(str, 250, TEXT("%d"), pnmLVScroll->dy);
				SendMessage(hList, LB_ADDSTRING, 0, (LPARAM) str); 
				scrlDelay = min(150, 150 / abs(pnmLVScroll->dy/2));
				for (i = 0; i < abs(pnmLVScroll->dy/2); i++)
				{
					Sleep(min(scrlDelay, 150));
					ListView_Scroll(hLV, 0, pnmLVScroll->dy*19);
					scrlDelay = (int)(scrlDelay * 1.25f);
				}
				scrolling = 0;
			}
		}
		if (lParam && ((LPNMHDR)lParam)->code == NM_CUSTOMDRAW)
		{
			LPNMLVCUSTOMDRAW lpNMCustomDraw = (LPNMLVCUSTOMDRAW) lParam;
			switch (lpNMCustomDraw->nmcd.dwDrawStage)
			{
			//case CDDS_POSTERASE:
			//	break;
			//case CDDS_POSTPAINT:
			//	break;
			//case CDDS_PREERASE:
			//	break;
			case CDDS_PREPAINT:
				// This event happens when the control has to be redrawn, 
				// to redraw all the subitems, we need to return the following:
				return (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW);
				break;
			//case CDDS_ITEM:
			//	break;
			//case CDDS_ITEMPOSTERASE:
			//	break;
			case CDDS_ITEMPOSTPAINT:
				// This event happens after Windows is done repainting the control.
				// If this is an item, go into the "IF"
				if (lpNMCustomDraw->dwItemType == LVCDI_ITEM)
				{
					TCHAR str[1000];
					RECT rc;
					HBRUSH hbr;

					// Get current item's rect.
					ListView_GetItemRect(lpNMCustomDraw->nmcd.hdr.hwndFrom, lpNMCustomDraw->nmcd.dwItemSpec, &rc, LVIR_BOUNDS);
					// Get current item's text
					ListView_GetItemText(lpNMCustomDraw->nmcd.hdr.hwndFrom, lpNMCustomDraw->nmcd.dwItemSpec, 0, str, 1000);
					// If item is being hovered on, draw a differet colored rect around it.
					// Else draw it with blue-ish background
					if (lpNMCustomDraw->nmcd.uItemState & CDIS_HOT)
					{
						hbr = CreateSolidBrush(RGB(212, 238, 249));
						FillRect(lpNMCustomDraw->nmcd.hdc, &rc, hbr);
						DeleteObject(hbr);
						hbr = CreateSolidBrush(RGB(228, 244, 254));
						rc.bottom--; rc.left++; rc.right--; rc.top++;
						ListView_Scroll(lpNMCustomDraw->nmcd.hdr.hwndFrom, 0, 1);
					}
					else
						hbr = CreateSolidBrush(RGB(223, 238, 244));
					FillRect(lpNMCustomDraw->nmcd.hdc, &rc, hbr);
					// Free sources.
					DeleteObject(hbr);
					// Print text to item's DC.
					TextOut(lpNMCustomDraw->nmcd.hdc, rc.left, rc.top, str, _tcslen(str));
				}
				break;
			//case CDDS_ITEMPREERASE:
			//	break;
			case CDDS_ITEMPREPAINT:
				// Make sure CDDS_ITEMPOSTPAINT occurs.
				return (CDRF_NOTIFYPOSTPAINT);
				break;
			//case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
			//	//return (CDRF_NOTIFYPOSTPAINT);
			//	break;
			//case CDDS_SUBITEM | CDDS_ITEMPOSTPAINT:
			//	break;
			//default:
			//	return CDRF_DODEFAULT;
			//	break;
			}
		}
		
		break;
	case WM_MOUSEMOVE:
		{
			// If we were in the but now exited it's borders redraw it as "Inactive"
			// Code description is the same as BtnProc function.
			if (inButton)
			{
				DRAWITEMSTRUCT drawItem = {0};
				inButton = 0;
				drawItem.CtlType = ODT_BUTTON;
				drawItem.CtlID = 0;
				drawItem.itemID = 0;
				drawItem.itemAction = ODA_FOCUS;
				drawItem.hwndItem = hBtn;
				drawItem.hDC = GetDC(hBtn);
				GetClientRect(hBtn, &drawItem.rcItem);
				SendMessage(hWnd, WM_DRAWITEM, 0, (LPARAM)(LPDRAWITEMSTRUCT)&drawItem);
				ReleaseDC(hBtn, drawItem.hDC);
			}

		}
		break;
	case WM_DRAWITEM:
		{
			//LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;

			//// Make sure that our control is a button
			//if (lpDrawItem->CtlType == ODT_BUTTON)
			//{
			//	// If the button has focus or is highlighted, draw load the "Active" image.
			//	// Else load the "Inactive" image.
			//	if (((lpDrawItem->itemState & ODS_FOCUS) || (lpDrawItem->itemState & ODS_HOTLIGHT)))
			//	{
			//		inButton = 1;
			//		setEditButtonImage(NULL, lpDrawItem->hDC, IDB_ON);
			//	}
			//	else
			//	{
			//		inButton = 0;
			//		setEditButtonImage(NULL, lpDrawItem->hDC, IDB_OFF);
			//	}
			//}

			//return TRUE;
		}
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (getEditButtonControlId(wmId))
		{
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
	case WM_PAINT:
		{
			HBITMAP hbmpOld, hbmpImage;
			HDC hDCMem;
			RECT rect;

			hdc = BeginPaint(hWnd, &ps);

			// Create a DC in memory, compatible with the button's original DC.
			hDCMem = CreateCompatibleDC(hdc);
			// Load the selected image from the resource file.
			hbmpImage = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_IFONE_BG));
			// Select the image into the DC. Keep a reference to the old bitmap.
			hbmpOld = (HBITMAP)SelectObject(hDCMem, hbmpImage);
			// Gets the dimensions of the button
			GetClientRect(hWnd, &rect);
			// Copies the bitmap from the memory DC into the buttons DC 
			BitBlt(hdc, 0, 0, rect.right, rect.bottom, hDCMem, 0, 0, SRCCOPY);
			// Select the original memory DC's bitmap.
			SelectObject(hDCMem, hbmpOld);
			// Free resources.
			DeleteDC(hDCMem);
			DeleteObject(hbmpImage);

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

			CreateGUI(hWnd, hInst);
			btn = createHoverButton(hWnd, hInst, 505, 165, 178, 178, 1, IDB_ON, IDB_OFF, "Test text");
			setHoverButtonTextColor(btn, 255);
			
			setHoverButtonFont(createHoverButton(hWnd, hInst, 505+178, 155, 178, 178, 2, IDB_MAIN_WND_CLOCK_ON, IDB_MAIN_WND_CLOCK_OFF, "abcdefgh"),
				TEXT("Fixedsys Excelsior 3.01"), 24);

			setDefaultEditButtonProc(WndProc);
			editBtn = createEditButton(hWnd, hInst, 505, 165+200, 320, 44, 3, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, "Test text");




			// Create an Custom-Drawn list view control - see ListView.cpp for details.
			hLV = createListView(hWnd, hInst);
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
				addListViewItem(hLV, lpBuffer, num);
			}
			addListViewItem(hLV, TEXT("הצילו, המוח שלי נפטר!!"), TEXT("G"));
			addListViewItem(hLV, TEXT("A"), TEXT("G"));

			//// Define button size - not needed anymore...
			//btn1.left = 5; btn1.right = btn1.left + 178; btn1.top = 155; btn1.bottom = btn1.top + 177;
			//// Create an Owner-Drawn button and save it's handle.
			//hBtn = CreateWindowEx(0, TEXT("button"), TEXT("Test Button"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW | BS_NOTIFY, 
			//	btn1.left, btn1.top, btn1.right - btn1.left, btn1.bottom - btn1.top, hWnd, NULL, hInst, NULL);
			//// Keep a reference to the original Window Procedure of the button control.
			//defBtnProc = (WNDPROC)GetWindowLong(hBtn, GWL_WNDPROC);
			//// Set our own custom Window Procedure to handle the button events.
			//SetWindowLong(hBtn, GWL_WNDPROC, (LONG_PTR)BtnProc);

			// Creates a standard windows type button
			CreateWindowEx(0, TEXT("button"), TEXT("Test Button"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON| BS_NOTIFY, 
				btn1.left, btn1.top + btn1.bottom - btn1.top + 10, btn1.right - btn1.left, btn1.bottom - btn1.top, hWnd, NULL, hInst, NULL);
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

void CreateGUI(HWND hWnd, HINSTANCE hInstance)
{
	HoverButton *tempBtn;
	int x, y, width, height;

	hbTopBarSkype = createHoverButton(hWnd, hInstance, 67, 136, 320, 20, 0, IDB_TOP_BAR_SKYPE_OFF, IDB_TOP_BAR_SKYPE_OFF, NULL);
	lockHoverButtonImage(hbTopBarSkype, TRUE);
	hbMainUnderDateBg = createHoverButton(hWnd, hInstance, 67, 156, 320, 97, 0, IDB_MAIN_WND_UNDER_DATE_BG, IDB_MAIN_WND_UNDER_DATE_BG, NULL);
	lockHoverButtonImage(hbMainUnderDateBg, TRUE);
	hbMainCenterPic = createHoverButton(hWnd, hInstance, 67, 253, 320, 273, 0, IDB_MAIN_WND_CENTER_PIC, IDB_MAIN_WND_CENTER_PIC, NULL);
	lockHoverButtonImage(hbMainCenterPic, TRUE);
	hbExitButton = createHoverButton(hWnd, hInstance, 193, 634, 69, 69, 0, IDB_EXIT_BUTTON_ON, IDB_EXIT_BUTTON_OFF, NULL);
	setHoverButtonAsPushButton(hbExitButton, TRUE);
	hwndContainerMain = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD/* | WS_VISIBLE*/, 67, 524, 320, 92, hWnd, NULL, hInstance, NULL);

	tempBtn = createHoverButton(hwndContainerMain, hInstance, 0, 0, 10, 92, 0, IDB_MAIN_WND_LOW_BAR_LEFT, IDB_MAIN_WND_LOW_BAR_LEFT, NULL);
	lockHoverButtonImage(tempBtn, TRUE);
	tempBtn = createHoverButton(hwndContainerMain, hInstance, 311, 0, 9, 92, 0, IDB_MAIN_WND_LOW_BAR_RIGHT, IDB_MAIN_WND_LOW_BAR_RIGHT, NULL);
	lockHoverButtonImage(tempBtn, TRUE);
	x = 10;
	y = 0;
	width = 75;
	height = 92;

	hbMainActionBtn[0] = createHoverButton(hwndContainerMain, hInstance, x, y, width, height, 0, IDB_MAIN_WND_CLOCK_ON, IDB_MAIN_WND_CLOCK_OFF, NULL);
	x += width;
	hbMainActionBtn[1] = createHoverButton(hwndContainerMain, hInstance, x, y, width, height, 0, IDB_MAIN_WND_CONTACT_ON, IDB_MAIN_WND_CONTACT_OFF, NULL);
	x += width;
	hbMainActionBtn[2] = createHoverButton(hwndContainerMain, hInstance, x, y, width+1, height, 0, IDB_MAIN_WND_INFO_ON, IDB_MAIN_WND_INFO_OFF, NULL);
	x += width+1;
	hbMainActionBtn[3] = createHoverButton(hwndContainerMain, hInstance, x, y, width, height, 0, IDB_MAIN_WND_BIN_EMPTY_ON, IDB_MAIN_WND_BIN_EMPTY_OFF, NULL);

	tempBtn = createHoverButton(hWnd, hInstance, 67, 550, 320, 66, 0, IDB_CONTACT_WND_BUTTON_BG, IDB_CONTACT_WND_BUTTON_BG, NULL);
	lockHoverButtonImage(tempBtn, TRUE);
	hwndContainerMisc = tempBtn->hButton;
	x = 5;
	y = 0;
	width = 70;
	height = 66;

	hbMainActionBtn[0] = createHoverButton(hwndContainerMisc, hInstance, x, y, width, height, 0, IDB_CONTACT_WND_BUTTON_CALL_ON, IDB_CONTACT_WND_BUTTON_CALL_OFF, NULL);
	x += width+10;
	hbMainActionBtn[1] = createHoverButton(hwndContainerMisc, hInstance, x, y, width, height, 0, IDB_CONTACT_WND_BUTTON_INFO_ON, IDB_CONTACT_WND_BUTTON_INFO_OFF, NULL);
	x += width+10;
	hbMainActionBtn[2] = createHoverButton(hwndContainerMisc, hInstance, x, y, width, height, 0, IDB_CONTACT_WND_BUTTON_EDIT_ON, IDB_CONTACT_WND_BUTTON_EDIT_OFF, NULL);
	x += width+10;
	hbMainActionBtn[3] = createHoverButton(hwndContainerMisc, hInstance, x, y, width, height, 0, IDB_CONTACT_WND_BUTTON_DEL_ON, IDB_CONTACT_WND_BUTTON_DEL_OFF, NULL);
}