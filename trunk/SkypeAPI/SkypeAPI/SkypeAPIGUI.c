// SkypeAPI.cpp : Defines the entry point for the application.
//

#pragma comment (lib, "comctl32.lib")

#include "stdafx.h"
#include "SkypeAPIGUI.h"
#include "SkypeAPI.h"
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
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

BOOL processSkypeMessage(WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	HACCEL hAccelTable;
	MSG msg;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SKYPEAPI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SKYPEAPI));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SKYPEAPI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SKYPEAPI);
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
HWND hList, hList2;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), TRUE);
		hList = CreateWindowEx(0, TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL, 0, 0, 600, 480, hWnd, NULL, hInst, NULL);
		SendMessage(hList, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), TRUE);
		hList2 = CreateWindowEx(0, TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL, 602, 0, 600, 480, hWnd, NULL, hInst, NULL);
		SendMessage(hList, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), TRUE);
		connectSkype(hWnd);
		break;
	case WM_COPYDATA:
		if (processSkypeMessage(wParam, lParam))
			return TRUE;
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			{
				COPYDATASTRUCT copyData = {0};
				copyData.dwData = 0;
				copyData.lpData = "CALL echo123";
				copyData.cbData = strlen("CALL echo123") + 1;
				SendMessage(getSkypeApiWindowHandle(), WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&copyData);
			}
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		if (processAttachmentMessage(message, wParam, lParam))
		{
			TCHAR str[200];
			switch(lParam)
			{
			case ATTACH_SUCCESS:
				_tcscpy_s(str, 200, TEXT("Successfully connected with Skype"));
				break;
			case ATTACH_PENDING:
				_tcscpy_s(str, 200, TEXT("Pending authorization from Skype"));
				break;
			case ATTACH_REFUSED:
				_tcscpy_s(str, 200, TEXT("Connection refused"));
				break;
			case ATTACH_NOT_AVAILABLE:
				_tcscpy_s(str, 200, TEXT("Connection is unavailable"));
				break;
			case ATTACH_AVAILABLE:
				_tcscpy_s(str, 200, TEXT("Connection is now available"));
				break;
			}
			SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)str);
			return TRUE;
		}
		else
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

				//COPYDATASTRUCT oCopyData;

				//// send command to skype
				//oCopyData.dwData=0;
				//oCopyData.lpData=acInputRow;
				//oCopyData.cbData=strlen(acInputRow)+1;
				//if( oCopyData.cbData!=1 )
				//	{
				//	if( SendMessage( hGlobal_SkypeAPIWindowHandle, WM_COPYDATA, (WPARAM)hInit_MainWindowHandle, (LPARAM)&oCopyData)==FALSE )
				//		{
				//		hGlobal_SkypeAPIWindowHandle=NULL;
				//		printf("!!! Disconnected\n");
				//		}
				//	}
				//}

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

BOOL processSkypeMessage(WPARAM wParam, LPARAM lParam)
{
	BOOL ret = FALSE;
	SkypeObject *skypeObject = NULL;

	if (translateSkypeMessage(wParam, lParam, &skypeObject))
	{
		if (skypeObject)
		{
			switch (skypeObject->object)
			{
			case OBJECT_CALL:
				{
					SkypeCallObject *callObject = (SkypeCallObject*)skypeObject;
					callObject->callId = callObject->callId;
				}
				break;
			}
			free(skypeObject);
		}
		ret = TRUE;
	}

	return ret;
}