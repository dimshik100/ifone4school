// iPhoneBook.cpp : Defines the entry point for the application.
//
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\ 
"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "shell32.lib.lib")

#include "stdafx.h"
#include "iPhoneBook.h"
#include "ListView.h"
#include "EditButton.h"
#include "HoverButton.h"
#include "ScrollContainer.h"
#include "Clock.h"
#include "SkypeHandler.h"
#include "PhoneBook.h"
#include "Miscellaneous.h"
#include "SkypeAPI.h"
#include "shellapi.h"
#include <commctrl.h>

#define MAX_LOADSTRING 100
#define TIMER_ID 10000
#define CLOCK_TIMER_ID TIMER_ID + 1
#define	PWRBTN_TIMER_ID TIMER_ID + 2
#define END_CALL_TIMER_ID TIMER_ID + 3

#define CONTROL_ID 20
#define BUTTON_ID_PWR				(CONTROL_ID + 0)
#define BUTTON_ID_CLOCK				(CONTROL_ID + 1)
#define BUTTON_ID_CONTACTS			(CONTROL_ID + 2)
#define BUTTON_ID_INFO				(CONTROL_ID + 3)
#define BUTTON_ID_BIN				(CONTROL_ID + 4)
#define BUTTON_ID_MISC1				(CONTROL_ID + 5)
#define BUTTON_ID_MISC2				(CONTROL_ID + 6)
#define BUTTON_ID_MISC3				(CONTROL_ID + 7)
#define BUTTON_ID_MISC4				(CONTROL_ID + 8)
#define BUTTON_ID_YES				(CONTROL_ID + 9)
#define BUTTON_ID_NO				(CONTROL_ID + 10)
#define BUTTON_ID_END_CALL			(CONTROL_ID + 11)
#define INFO_ID_LAST_NAME			(CONTROL_ID + 12)
#define INFO_ID_FIRST_NAME			(CONTROL_ID + 13)
#define INFO_ID_PHONE				(CONTROL_ID + 14)
#define INFO_ID_WEB					(CONTROL_ID + 15)
#define INFO_ID_EMAIL				(CONTROL_ID + 16)
#define INFO_ID_AGE					(CONTROL_ID + 17)
#define INFO_ID_ADDRESS_CNT			(CONTROL_ID + 18)
#define INFO_ID_ADDRESS_CTY			(CONTROL_ID + 19)
#define INFO_ID_ADDRESS_STR			(CONTROL_ID + 20)
#define INFO_ID_ADDRESS_NUM			(CONTROL_ID + 21)
#define INFO_ID_ADDRESS				(CONTROL_ID + 22)
#define INFO_ID_SKYPE				(CONTROL_ID + 23)
#define BUTTON_ID_ALL_CONTACTS		(CONTROL_ID + 24)
#define BUTTON_ID_EDIT_CONTACT		(CONTROL_ID + 25)
#define BUTTON_ID_EDIT_SAVE_CONTACT	(CONTROL_ID + 26)
#define BUTTON_ID_ADD_CONTACT_EMPTY		(CONTROL_ID + 27)
#define BUTTON_ID_EMPTY_TRASH		(CONTROL_ID + 28)
#define BUTTON_ID_REOVER_CONTACT	(CONTROL_ID + 29)
#define LV_CONTACTS_ID				(CONTROL_ID + 30)
#define EDIT_ID_SEARCH				(CONTROL_ID + 31)
#define BUTTON_ID_GO_TO_WEBSITE		(CONTROL_ID + 32)
#define BUTTON_ID_SEND_EMAIL		(CONTROL_ID + 33)
#define BUTTON_ID_SEE_ON_MAP		(CONTROL_ID + 34)
#define BUTTON_ID_SKYPE_HANDLE		(CONTROL_ID + 35)

typedef enum {	SCREEN_MAIN, SCREEN_CONTACTS, SCREEN_MEM_INFO, SCREEN_TRASH, SCREEN_CALL_MODE, 
				SCREEN_CONTACT_INFO, SCREEN_CONTACT_ADD, SCREEN_CONTACT_EDIT, SCREEN_CLOCK }
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
VOID CALLBACK		EndCallTimerProc(HWND, UINT, UINT_PTR, DWORD);
LRESULT CALLBACK	TransparentLabelProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	ContainerProc(HWND, UINT, WPARAM, LPARAM);
void CALLBACK		skypeCallStatusCallback(SkypeCallObject *skypeCallObject);
void CALLBACK		skypeConnectionStatusCallback(SkypeApiInitStatus skypeApiInitStatus);

void createGUI(HWND hWnd, HINSTANCE hInstance);
void showChildContainers(ScreenMode screen);
void enableChildContainers(BOOL value);
void fillContactDetails(Contact *contact, int inEditMode);
void fillEditContactDetails(Contact *contact);
int saveContactDetails(Contact *contact);


HoverButton 
		*hbTopBarSkype, *hbMainUnderDateBg, *hbMainCenterPic, *hbExitButton, *hbContainerCall,
		*hbMainActionBtn[4], *hbMiscActionBtn[4], *hbYes, *hbNo, *hbAllContacts, *hbEditSaveContact,
		*hbAddContactEmpty, *hbRecoverContact, *hbContactsSearchBg, *hbContactsTitleBg, *hbContactInfoTitleBg,
		*hbEmail, *hbGoogleMap, *hbSkypeHandle, *hbWebsite, *hbClock;

HWND		hlblContactInfo[11];
EditButton	*ebContactInfo[11];
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

HWND hwndContainerMainButtons, hwndContainerMiscButtons, hwndContainerContacts, hwndContainerContactDetails;
HWND hwndSearchBox, hwndConfirmDialog, hwndMain, hwndScrollContainer;
HWND hLV;
WNDPROC defContainerProc, defTransparentLabelProc;
HFONT digitalClockFont;
TCHAR digitalClockFontPath[MAX_PATH];
const RECT ifoneScreenRect = { 67, 136, 386, 615 };
ScreenMode screenMode = SCREEN_MAIN;
int isConfirmOn = FALSE;
SkypeCallObject currentCall;

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
	wcex.lpszMenuName	= NULL;
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
	RECT rcSize = { 0, 0, 450, 750 };
	SIZE size;

	hInst = hInstance; // Store instance handle in our global variable

	AdjustWindowRect(&rcSize, ((WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN) & ~WS_OVERLAPPED, FALSE);
	size.cx = rcSize.right - rcSize.left;
	size.cy = rcSize.bottom - rcSize.top;
	hwndMain = hWnd = CreateWindowEx(0, szWindowClass, szTitle, (WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, size.cx, size.cy, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}
	//makeWindowTransparentByMask(hWnd, IDB_IPHONE_BG_MASK);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

void CALLBACK skypeCallStatusCallback(SkypeCallObject *skypeCallObject)
{
	if (skypeCallObject && skypeCallObject->object == OBJECT_CALL)
	{
		TCHAR str[256] = {0};
		TCHAR statusStr[50] = {0}, strDuration[25] = {0}, strContactName[MAX_FNAME+MAX_LNAME];
		DynamicListC contactList;

		switch (skypeCallObject->status)
		{
		case CALLSTATUS_ROUTING:
		case CALLSTATUS_RINGING:
			{
				showChildContainers(SCREEN_CALL_MODE);
				enableChildContainers(FALSE);
				EnableWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), TRUE);
				if (skypeCallObject->type == CALLTYPE_INCOMING_P2P || skypeCallObject->type == CALLTYPE_INCOMING_PSTN)
					_tcscpy_s(statusStr, 50, TEXT("Incoming call"));
				else
					_tcscpy_s(statusStr, 50, TEXT("Calling..."));
			}
			break;
		case CALLSTATUS_IN_PROGRESS:
			_tcscpy_s(statusStr, 50, TEXT("Call online"));
			break;
		case CALLSTATUS_FINISHED:
			_tcscpy_s(statusStr, 50, TEXT("Call ended"));
			_stprintf_s(strDuration, 25, TEXT("Duration: %02d:%02d:%02d"), skypeCallObject->duration / 3600, (skypeCallObject->duration % 3600) / 60, (skypeCallObject->duration % 60));
			SetTimer(getHoverButtonHwnd(hbContainerCall), END_CALL_TIMER_ID, 5000, EndCallTimerProc);
			break;
		case CALLSTATUS_CANCELLED:
		case CALLSTATUS_FAILED:
		case CALLSTATUS_MISSED:
		case CALLSTATUS_REFUSED:
			_tcscpy_s(statusStr, 50, TEXT("Call ended"));
			_stprintf_s(strDuration, 25, TEXT("Duration: %02d:%02d:%02d"), skypeCallObject->duration / 3600, (skypeCallObject->duration % 3600) / 60, (skypeCallObject->duration % 60));
			SetTimer(getHoverButtonHwnd(hbContainerCall), END_CALL_TIMER_ID, 5000, EndCallTimerProc);
			break;
		}		
		switch (skypeCallObject->property)
		{
		case CALLPROPERTY_DURATION:
			_stprintf_s(strDuration, 25, TEXT("Duration: %02d:%02d:%02d"), skypeCallObject->duration / 3600, (skypeCallObject->duration % 3600) / 60, (skypeCallObject->duration % 60));
			break;
		case CALLPROPERTY_STATUS:
			break;
		default:
			//strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("Other property notification\n"), skypeCallObject->property);
			break;
		}

		contactList = getContactListInitiated();
		if (contactList)
		{
			Contact *contact = NULL;
			for (listSelectFirst(contactList); listSelectCurrent(contactList); listSelectNext(contactList, NULL))
			{
				listGetValue(contactList, NULL, &contact);
				if (!_tcsicmp(contact->skypeName, skypeCallObject->partnerHandle))
				{
					_stprintf_s(strContactName, MAX_FNAME+MAX_LNAME, TEXT("%s %s"), contact->firstName, contact->lastName);
					break;
				}
			}
			if (_tcslen(strContactName) == 0)
				_tcscpy_s(strContactName, MAX_FNAME+MAX_LNAME, TEXT("Unknown"));
		}

		//strPos +=_stprintf_s(str+strPos, 256-strPos, TEXT("skypeCallObject = 0x%p"), skypeCallObject);
		if (_tcslen(strDuration) > 0)
			_stprintf_s(str, 256, TEXT("%s\n\n%s\n\n%s\n%s"), statusStr, strDuration, strContactName, skypeCallObject->partnerDisplayName);
		else
			_stprintf_s(str, 256, TEXT("%s\n\n%s\n%s"), statusStr, strContactName, skypeCallObject->partnerDisplayName);
		currentCall = *skypeCallObject;
		setHoverButtonText(hbContainerCall, str);
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
		lockHoverButtonImage(hbSkypeHandle, TRUE);
		break;
	// Skype defined constants
	case ATTACH_AVAILABLE:
		break;
	case ATTACH_NOT_AVAILABLE:
		setHoverButtonStateImages(hbTopBarSkype, IDB_TOP_BAR_SKYPE_OFF, IDB_TOP_BAR_SKYPE_OFF);
		lockHoverButtonImage(hbSkypeHandle, TRUE);
		break;
	case ATTACH_PENDING:
		break;
	case ATTACH_REFUSED:
		setHoverButtonStateImages(hbTopBarSkype, IDB_TOP_BAR_SKYPE_OFF, IDB_TOP_BAR_SKYPE_OFF);
		lockHoverButtonImage(hbSkypeHandle, TRUE);
		break;
	case ATTACH_SUCCESS:
		setHoverButtonStateImages(hbTopBarSkype, IDB_TOP_BAR_SKYPE_ON, IDB_TOP_BAR_SKYPE_ON);
		lockHoverButtonImage(hbSkypeHandle, FALSE);
		break;
	}
}

void fillContactDetails(Contact *contact, int inEditMode)
{
	int fieldCount = 0;
	int i;
	SIZE size = { 320, 350 }, virtSize;
	POINT pt;
	
	virtSize.cx = 320 - GetSystemMetrics(SM_CXVSCROLL);
	// Initiate fields
	for (i = 10; i >= 0; i--)	// Go in oposite direction so that the 
	{							// last editbutton to be set is the first in the array
		getChildInParentOffset(getEditButtonHwnd(ebContactInfo[i]), &pt);
		MoveWindow(getEditButtonHwnd(ebContactInfo[i]), pt.x, pt.y, virtSize.cx, 44, FALSE);
		setEditButtonEditStyles(ebContactInfo[i], 0);
		if (inEditMode)
		{
			lockEditButton(ebContactInfo[i], FALSE);
			showEditButtonEdit(ebContactInfo[i], TRUE);
		}
		else
		{
			showEditButtonEdit(ebContactInfo[i], FALSE);
			lockEditButton(ebContactInfo[i], TRUE);
		}
	}

	ShowWindow(getHoverButtonHwnd(hbSkypeHandle), SW_HIDE);
	ShowWindow(getHoverButtonHwnd(hbEmail), SW_HIDE);
	ShowWindow(getHoverButtonHwnd(hbGoogleMap), SW_HIDE);
	ShowWindow(getHoverButtonHwnd(hbWebsite), SW_HIDE);

	setEditButtonText(ebContactInfo[fieldCount++], contact->firstName);	//TEXT("First name")
	setEditButtonText(ebContactInfo[fieldCount++], contact->lastName);	//TEXT("Last name")
	setEditButtonText(ebContactInfo[fieldCount], contact->phone);		//TEXT("Phone number")
		setEditButtonEditStyles(ebContactInfo[fieldCount++], ES_NUMBER);// Set text field to numeric.
	if (_tcslen(contact->skypeName) > 0 || inEditMode)
	{
		if (!inEditMode)
		{
			getChildInParentOffset(getEditButtonHwnd(ebContactInfo[fieldCount]), &pt);
			MoveWindow(getEditButtonHwnd(ebContactInfo[fieldCount]), pt.x, pt.y, virtSize.cx - 44, 44, FALSE);
			MoveWindow(getHoverButtonHwnd(hbSkypeHandle), virtSize.cx - 43, pt.y + 1, 42, 42, FALSE);
			ShowWindow(getHoverButtonHwnd(hbSkypeHandle), SW_SHOW);
		}

		SetWindowText(hlblContactInfo[fieldCount], TEXT("Skype handle"));
		setEditButtonText(ebContactInfo[fieldCount++], contact->skypeName);
	}
	if (_tcslen(contact->email) > 0 || inEditMode)
	{
		if (!inEditMode)
		{
			getChildInParentOffset(getEditButtonHwnd(ebContactInfo[fieldCount]), &pt);
			MoveWindow(getEditButtonHwnd(ebContactInfo[fieldCount]), pt.x, pt.y, virtSize.cx - 44, 44, FALSE);
			MoveWindow(getHoverButtonHwnd(hbEmail), virtSize.cx - 43, pt.y + 1, 42, 42, FALSE);
			ShowWindow(getHoverButtonHwnd(hbEmail), SW_SHOW);
		}

		SetWindowText(hlblContactInfo[fieldCount], TEXT("E-mail"));
		setEditButtonText(ebContactInfo[fieldCount++], contact->email);
	}
	if (_tcslen(contact->webSite) > 0 || inEditMode)
	{
		if (!inEditMode)
		{
			getChildInParentOffset(getEditButtonHwnd(ebContactInfo[fieldCount]), &pt);
			MoveWindow(getEditButtonHwnd(ebContactInfo[fieldCount]), pt.x, pt.y, virtSize.cx - 44, 44, FALSE);
			MoveWindow(getHoverButtonHwnd(hbWebsite), virtSize.cx - 43, pt.y + 1, 42, 42, FALSE);
			ShowWindow(getHoverButtonHwnd(hbWebsite), SW_SHOW);
		}

		SetWindowText(hlblContactInfo[fieldCount], TEXT("Web site"));
		setEditButtonText(ebContactInfo[fieldCount++], contact->webSite);
	}
	if (contact->age > 0 || inEditMode)
	{
		SetWindowText(hlblContactInfo[fieldCount], TEXT("Age"));
		setEditButtonEditStyles(ebContactInfo[fieldCount], ES_NUMBER);
		if (!inEditMode)
		{
			TCHAR ageStr[20];
			_stprintf_s(ageStr, 20, TEXT("%d"), contact->age);
			setEditButtonText(ebContactInfo[fieldCount++], ageStr);
		}
		else
			setEditButtonText(ebContactInfo[fieldCount++], TEXT(""));
	}
	if (_tcslen(contact->address.country) > 0 || inEditMode)
	{
		if (!inEditMode)
		{
			getChildInParentOffset(getEditButtonHwnd(ebContactInfo[fieldCount]), &pt);
			MoveWindow(getEditButtonHwnd(ebContactInfo[fieldCount]), pt.x, pt.y, virtSize.cx - 44, 44, FALSE);
			MoveWindow(getHoverButtonHwnd(hbGoogleMap), virtSize.cx - 43, pt.y + 1, 42, 42, FALSE);
			ShowWindow(getHoverButtonHwnd(hbGoogleMap), SW_SHOW);
		}

		SetWindowText(hlblContactInfo[fieldCount], TEXT("Country"));
		setEditButtonText(ebContactInfo[fieldCount++], contact->address.country);
	}
	if (_tcslen(contact->address.city) > 0 || inEditMode)
	{
		if (!inEditMode && !ShowWindow(getHoverButtonHwnd(hbGoogleMap), SW_SHOW))
		{
			getChildInParentOffset(getEditButtonHwnd(ebContactInfo[fieldCount]), &pt);
			MoveWindow(getEditButtonHwnd(ebContactInfo[fieldCount]), pt.x, pt.y, virtSize.cx - 44, 44, FALSE);
			MoveWindow(getHoverButtonHwnd(hbGoogleMap), virtSize.cx - 43, pt.y + 1, 42, 42, FALSE);
		}

		SetWindowText(hlblContactInfo[fieldCount], TEXT("City"));
		setEditButtonText(ebContactInfo[fieldCount++], contact->address.city);
	}
	if (_tcslen(contact->address.street) > 0 || inEditMode)
	{
		SetWindowText(hlblContactInfo[fieldCount], TEXT("Street"));
		setEditButtonText(ebContactInfo[fieldCount++], contact->address.street);
	}
	if (_tcslen(contact->address.number) > 0 || inEditMode)
	{
		SetWindowText(hlblContactInfo[fieldCount], TEXT("Number"));
		setEditButtonText(ebContactInfo[fieldCount++], contact->address.number);
	}

	for (i = 0; i < fieldCount; i++)
	{
		ShowWindow(getEditButtonHwnd(ebContactInfo[i]), SW_SHOW);
		ShowWindow(hlblContactInfo[i], SW_SHOW);
	}
	for (i = fieldCount; i< 11; i++)
	{
		ShowWindow(getEditButtonHwnd(ebContactInfo[i]), SW_HIDE);
		ShowWindow(hlblContactInfo[i], SW_HIDE);
	}

	// Calculate and set new scroll range for the contact details container.
	virtSize.cy = (64 * fieldCount) + (350 % 64);
	virtSize.cx = 320 - GetSystemMetrics(SM_CXVSCROLL);
	setScrollContainerSize(hwndScrollContainer, &size, &virtSize);
	InvalidateRect(hwndScrollContainer, NULL, FALSE);
}

int saveContactDetails(Contact *contact)
{
	int fieldCount = 0, i;
	TCHAR buff[50];
	
	for (i = 0; i < 3; i++)
	{
		getEditButtonText(ebContactInfo[i], buff, 50, FALSE);
		if (_tcslen(buff) == 0)
			return FALSE;
		_tcscpy_s(buff, 50, TEXT(""));
	}
	// Only update text if value is saved in the editbutton
	getEditButtonText(ebContactInfo[fieldCount++], contact->firstName, MAX_FNAME-1, FALSE);
	getEditButtonText(ebContactInfo[fieldCount++], contact->lastName, MAX_LNAME-1, FALSE);
	getEditButtonText(ebContactInfo[fieldCount++], contact->phone, MAX_PHONE-1, FALSE);
	getEditButtonText(ebContactInfo[fieldCount++], contact->skypeName, MAX_SKYPE-1, FALSE);
	getEditButtonText(ebContactInfo[fieldCount++], contact->email, MAX_EMAIL-1, FALSE);
	getEditButtonText(ebContactInfo[fieldCount++], contact->webSite, MAX_SITE-1, FALSE);
	{
		TCHAR ageStr[20];
		getEditButtonText(ebContactInfo[fieldCount++], ageStr, 20, FALSE);
		contact->age = _tstoi(ageStr);
	}
	getEditButtonText(ebContactInfo[fieldCount++], contact->address.country, MAX_COUNTRY-1, FALSE);
	getEditButtonText(ebContactInfo[fieldCount++], contact->address.city, MAX_CITY-1, FALSE);
	getEditButtonText(ebContactInfo[fieldCount++], contact->address.street, MAX_STREET-1, FALSE);
	getEditButtonText(ebContactInfo[fieldCount++], contact->address.number, MAX_NUMBER-1, FALSE);

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

LRESULT CALLBACK TransparentLabelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_ERASEBKGND)
		return TRUE;

	return CallWindowProc(defTransparentLabelProc, hWnd, message, wParam, lParam);
}

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
			// Clock button handler
			case BUTTON_ID_CLOCK:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP && screenMode != SCREEN_CLOCK)
				{
					showChildContainers(SCREEN_CLOCK);
					//if (isOsVista())
					//{
					//// If we support transparent confirm dialog, we must set it's position
					//// to be centered on the ifonebook screen
					//	POINT pt = {0, 0};
					//	ClientToScreen(GetParent(hWnd), &pt);
					//	MoveWindow(hwndConfirmDialog, 
					//		pt.x + ifoneScreenRect.left + (ifoneScreenRect.right - ifoneScreenRect.left - 277) / 2,
					//		pt.y + ifoneScreenRect.top + (ifoneScreenRect.bottom - ifoneScreenRect.top - 103) / 2,
					//		277, 103, TRUE);
					//}
					//ShowWindow(hwndConfirmDialog, SW_SHOW);
					//isConfirmOn = TRUE;
					//enableChildContainers(FALSE);
					//screenMode = SCREEN_CLOCK;
				}
				break;
			// View all contacts button(s) handler
			case BUTTON_ID_CONTACTS:
			case BUTTON_ID_ALL_CONTACTS:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					SetWindowText(hwndSearchBox, TEXT(""));
					fillListView(hLV, getContactListInitiated(), NULL);
					SetFocus(hwndSearchBox);
					showChildContainers(SCREEN_CONTACTS);
				}
				break;
			case BUTTON_ID_INFO:
				break;
			case BUTTON_ID_BIN:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					fillListView(hLV, getTrashListFromFile(), NULL);
					showChildContainers(SCREEN_TRASH);
				}
				break;
			case BUTTON_ID_ADD_CONTACT_EMPTY:
				// In "All contacts" screen
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP && screenMode == SCREEN_CONTACTS)
				{
					Contact contact = {0};
					// Replace to "Save" image.
					setHoverButtonStateImages(hbEditSaveContact, IDB_CONTACT_INFO_ALL_CONTACTS, IDB_CONTACT_INFO_ALL_CONTACTS);
					fillContactDetails(&contact, TRUE);
					showChildContainers(SCREEN_CONTACT_ADD);
				}
				// "Recycle bin" screen
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP && screenMode == SCREEN_TRASH)
				{
					// Are you sure dialog here
					emptyTrashList();
					freeTrashListLocal();
					// Return to main screen
					SendMessage(hwndMain, WM_COMMAND, (LPARAM)MAKELONG(BUTTON_ID_PWR, HOVER_BUTTON_LMOUSE_UP), 0);
				}
				break;
			case BUTTON_ID_REOVER_CONTACT:
				// "Recycle bin" screen
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP && screenMode == SCREEN_TRASH)
				{
					Contact *contact = (Contact*)getListViewSelectedItemParam(hLV);
					if (contact)
					{
						if (recoverContact(contact->index))
						{
							freeContactListLocal();
							// Reload the "Recycle bin" screen
							SendMessage(hwndContainerMainButtons, message, MAKELONG(BUTTON_ID_BIN, wmEvent), 0);
						}
					}
				}
				break;
			case BUTTON_ID_EDIT_SAVE_CONTACT:
				// If clicked Save in contact view
				// Else if clicked Edit
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP && screenMode == SCREEN_CONTACT_EDIT)
				{
					Contact *contact = (Contact*)getListViewSelectedItemParam(hLV);
					if (saveContactDetails(contact))
					{
						setHoverButtonStateImages(hbEditSaveContact, IDB_CONTACT_INFO_EDIT_CONTACT, IDB_CONTACT_INFO_EDIT_CONTACT);
						fillContactDetails(contact, FALSE);
						editContact(contact);
						showChildContainers(SCREEN_CONTACT_INFO);
						SetFocus(hwndScrollContainer);
					}
				}				
				else if (wmEvent == HOVER_BUTTON_LMOUSE_UP && screenMode == SCREEN_CONTACT_INFO)
					// Edit a contact
					SendMessage(hWnd, message, MAKELONG(BUTTON_ID_MISC3, wmEvent), 0);
				else if (wmEvent == HOVER_BUTTON_LMOUSE_UP && screenMode == SCREEN_CONTACT_ADD)
				{
					Contact contact = {0};
					if (saveContactDetails(&contact))
					{
						addContactToLocalList(&contact);
						addContact(&contact);
						// Return to contact list
						SendMessage(hWnd, message, MAKELONG(BUTTON_ID_ALL_CONTACTS, wmEvent), 0);
					}
				}
				break;
			// Miscellaneous buttons handlers
			case BUTTON_ID_MISC1:
				// Skype call
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP && (screenMode == SCREEN_CONTACT_INFO || screenMode == SCREEN_CONTACTS))
				{				
					Contact *contact = (Contact*)getListViewSelectedItemParam(hLV);
					if (contact && _tcslen(contact->skypeName) > 0)
						call(contact->skypeName);
				}
				break;
			case BUTTON_ID_MISC2:
				// Go to Single contact info screen
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP && screenMode != SCREEN_CONTACT_INFO)
				{
					Contact *contact = (Contact*)getListViewSelectedItemParam(hLV);
					if (contact)
					{
						setHoverButtonStateImages(hbEditSaveContact, IDB_CONTACT_INFO_EDIT_CONTACT, IDB_CONTACT_INFO_EDIT_CONTACT);
						fillContactDetails(contact, FALSE);
						showChildContainers(SCREEN_CONTACT_INFO);
						SetFocus(hwndScrollContainer);
					}
				}
				break;
			case BUTTON_ID_MISC3: 
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP && 
					(screenMode == SCREEN_CONTACT_INFO || screenMode == SCREEN_CONTACTS))
				{
					Contact *contact = (Contact*)getListViewSelectedItemParam(hLV);
					if (contact)
					{
						// Replace to "Save" image.
						setHoverButtonStateImages(hbEditSaveContact, IDB_CONTACT_INFO_ALL_CONTACTS, IDB_CONTACT_INFO_ALL_CONTACTS);
						fillContactDetails(contact, TRUE);
						showChildContainers(SCREEN_CONTACT_EDIT);
					}
				}
				break;
			case BUTTON_ID_MISC4:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP && 
					(screenMode == SCREEN_CONTACT_INFO || screenMode == SCREEN_CONTACTS))
				{
					Contact *contact = (Contact*)getListViewSelectedItemParam(hLV);
					if (contact)
					{
						if (deleteContact(contact->index))
						{
							freeContactListLocal();
							// Return to contact list
							SendMessage(hWnd, message, MAKELONG(BUTTON_ID_ALL_CONTACTS, wmEvent), 0);
						}
					}
				}
				break;
			// Confirmation dialog handler
			case BUTTON_ID_YES:
			case BUTTON_ID_NO:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					ShowWindow(hwndConfirmDialog, SW_HIDE);
					isConfirmOn = FALSE;
					enableChildContainers(TRUE);
				}
				break;
			case BUTTON_ID_END_CALL: // End current call.
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					hangup(currentCall.callId);
				}
			// Search edit-control handler
			case EDIT_ID_SEARCH:
				if (wmEvent == EN_CHANGE)
				{
					TCHAR str[256];
					GetWindowText((HWND)lParam, str, 256);
					fillListView(hLV, getContactListLocal(), str);
				}
				break;
			case BUTTON_ID_GO_TO_WEBSITE:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					Contact *contact = (Contact*)getListViewSelectedItemParam(hLV);
					if (contact)
					{
						// TODO: Handle errors.
						if ((unsigned int)ShellExecute(NULL, TEXT("open"), contact->webSite, TEXT(""), TEXT(""), SW_SHOWDEFAULT) > 32);
					}
				}
				break;
			case BUTTON_ID_SEND_EMAIL:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					Contact *contact = (Contact*)getListViewSelectedItemParam(hLV);
					if (contact)
					{
						TCHAR buff[MAX_EMAIL + 10];
						_stprintf_s(buff, MAX_EMAIL + 10, TEXT("mailto:%s"), contact->email);
						// TODO: Handle errors.
						if ((unsigned int)ShellExecute(NULL, TEXT("open"), buff, TEXT(""), TEXT(""), SW_SHOWDEFAULT) > 32);
					}
				}
				break;
			case BUTTON_ID_SEE_ON_MAP:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
				{
					Contact *contact = (Contact*)getListViewSelectedItemParam(hLV);
					if (contact)
					{
						TCHAR buff[MAX_COUNTRY+MAX_CITY+MAX_STREET+MAX_NUMBER + 30];
						_stprintf_s(buff, MAX_COUNTRY+MAX_CITY+MAX_STREET+MAX_NUMBER + 30,
							TEXT("http://maps.google.com/?q=%s,%s,%s,%s"),
							contact->address.country, contact->address.city, contact->address.street, contact->address.number);
						// TODO: Handle errors.
						if ((unsigned int)ShellExecute(NULL, TEXT("open"), buff, TEXT(""), TEXT(""), SW_SHOWDEFAULT) > 32);
					}
				}
				break;
			case BUTTON_ID_SKYPE_HANDLE:
				if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
					SendMessage(hWnd, message, MAKELONG(BUTTON_ID_MISC1, wmEvent), 0);
				break;
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
		}
		break;
	case WM_ERASEBKGND:
		return TRUE;
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
					showChildContainers(SCREEN_MAIN);
				KillTimer(hWnd, PWRBTN_TIMER_ID);
				// If timer successfully killed, means the pwr button was manually pressed, so release buttons.
				if (KillTimer(getHoverButtonHwnd(hbContainerCall), END_CALL_TIMER_ID))
					enableChildContainers(TRUE);
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
		deleteEditButtons();
		disconnectSkype(hInst);
		unloadCustomFont(TEXT("AtomicClockRadio.ttf"));
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		{
			createGUI(hWnd, hInst);
			setSkypeCallStatusCallback(skypeCallStatusCallback);
			setSkypeConnectionStatusCallback(skypeConnectionStatusCallback);
			connectSkype(hInst);
			SetTimer(hWnd, CLOCK_TIMER_ID, 500, (TIMERPROC)ClockTimerProc);			
			initListViewColumns(hLV);
			loadCustomFont(TEXT("AtomicClockRadio.ttf"));
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
		if (screenMode == SCREEN_MAIN || screenMode == SCREEN_CALL_MODE)
		{
			setHoverButtonText(hbMainUnderDateBg, str);
			getHoverButtonText(hbTopBarSkype, str, 1000);
			if (_tcslen(str) > 0)
				setHoverButtonText(hbTopBarSkype, NULL);
		}
		else if (screenMode != SCREEN_CLOCK)
			setHoverButtonText(hbTopBarSkype, str);
		else
		{
			_tcsftime(str, 1000, TEXT("%H:%M:%S\n%a\n%b %d\n%Y"), &today);
			setHoverButtonText(hbClock, str);
		}
	}
}

VOID CALLBACK		EndCallTimerProc(HWND hWnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
	UNREFERENCED_PARAMETER(hWnd), UNREFERENCED_PARAMETER(message), UNREFERENCED_PARAMETER(idEvent), UNREFERENCED_PARAMETER(dwTime);
	
	enableChildContainers(TRUE);
	SendMessage(hwndMain, WM_COMMAND, (LPARAM)MAKELONG(BUTTON_ID_PWR, HOVER_BUTTON_LMOUSE_UP), 0);
	KillTimer(getHoverButtonHwnd(hbContainerCall), END_CALL_TIMER_ID);
}

void showChildContainers(ScreenMode screen)
{
	switch(screen)
	{
	case SCREEN_MAIN:
		ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), SW_SHOW);
		ShowWindow(hwndContainerMiscButtons, SW_HIDE);
		AnimateWindow(hwndContainerMainButtons, 100, AW_VER_NEGATIVE | AW_ACTIVATE);
		AnimateWindow(getHoverButtonHwnd(hbMainCenterPic), 200, AW_CENTER | AW_ACTIVATE);
		ShowWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), SW_HIDE);
		ShowWindow(hwndContainerContacts, SW_HIDE);
		ShowWindow(hwndContainerContactDetails, SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbClock), SW_HIDE);
		break;
	case SCREEN_CONTACTS:
		setHoverButtonStateImages(hbContactsTitleBg, IDB_CONTACT_WND_APP_NAME, IDB_CONTACT_WND_APP_NAME);
		ShowWindow(hwndSearchBox, SW_SHOW);
		ShowWindow(getHoverButtonHwnd(hbContactsSearchBg), SW_SHOW);
		ShowWindow(getHoverButtonHwnd(hbEditSaveContact), SW_SHOW);
		ShowWindow(getHoverButtonHwnd(hbAllContacts), SW_SHOW);
		ShowWindow(getHoverButtonHwnd(hbRecoverContact), SW_HIDE);
		setHoverButtonStateImages(hbAddContactEmpty, IDB_CONTACT_ADD, IDB_CONTACT_ADD);
		{
			POINT pt;
			MoveWindow(hLV, 0, 88, 320, 306, TRUE);
			getChildInParentOffset(hwndContainerContacts, &pt);
			MoveWindow(hwndContainerContacts, pt.x, pt.y, 320, 394, TRUE);
			AnimateWindow(hwndContainerContacts, 400, AW_ACTIVATE | AW_HOR_NEGATIVE);
		}
		ShowWindow(getHoverButtonHwnd(hbMainCenterPic), SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), SW_HIDE);
		ShowWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), SW_HIDE);
		ShowWindow(hwndContainerContactDetails, SW_HIDE);
		AnimateWindow(hwndContainerMiscButtons, 150, AW_ACTIVATE | AW_VER_POSITIVE);
		ShowWindow(hwndContainerMainButtons, SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbClock), SW_HIDE);
		break;
	case SCREEN_TRASH:
		setHoverButtonStateImages(hbContactsTitleBg, IDB_TRASH_WND_TITLE, IDB_TRASH_WND_TITLE);
		ShowWindow(hwndSearchBox, SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbContactsSearchBg), SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbEditSaveContact), SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbAllContacts), SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbRecoverContact), SW_SHOW);
		setHoverButtonStateImages(hbAddContactEmpty, IDB_TRASH_BUTTON_EMPTY, IDB_TRASH_BUTTON_EMPTY);
		ShowWindow(hwndContainerMiscButtons, SW_HIDE);
		{
			POINT pt;
			MoveWindow(hLV, 0, 44, 320, 416, TRUE);
			getChildInParentOffset(hwndContainerContacts, &pt);
			MoveWindow(hwndContainerContacts, pt.x, pt.y, 320, 460, TRUE);
			AnimateWindow(hwndContainerContacts, 400, AW_ACTIVATE | AW_HOR_NEGATIVE);
		}
		ShowWindow(hwndContainerMainButtons, SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbMainCenterPic), SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), SW_HIDE);
		ShowWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), SW_HIDE);
		ShowWindow(hwndContainerContactDetails, SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbClock), SW_HIDE);
		break;
	case SCREEN_MEM_INFO:
		ShowWindow(getHoverButtonHwnd(hbMainCenterPic), SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), SW_HIDE);
		ShowWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), SW_HIDE);
		ShowWindow(hwndContainerMainButtons, SW_HIDE);
		ShowWindow(hwndContainerMiscButtons, SW_SHOW);
		ShowWindow(hwndContainerContacts, SW_HIDE);
		ShowWindow(hwndContainerContactDetails, SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbClock), SW_HIDE);
		break;
	case SCREEN_CALL_MODE:
		ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), SW_SHOW);
		ShowWindow(hwndContainerMainButtons, SW_SHOW);
		setHoverButtonText(hbContainerCall, NULL);
		AnimateWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), 100, AW_CENTER | AW_ACTIVATE);
		InvalidateRect(GetParent(getHoverButtonHwnd(hbContainerCall)), NULL, FALSE);
		ShowWindow(getHoverButtonHwnd(hbMainCenterPic), SW_HIDE);
		ShowWindow(hwndContainerMiscButtons, SW_HIDE);
		ShowWindow(hwndContainerContacts, SW_HIDE);
		ShowWindow(hwndContainerContactDetails, SW_HIDE);
		// Force redraw the control (AnimateWindow, improperly renders children of HoverButton control).
		ShowWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), SW_HIDE);
		ShowWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), SW_SHOW);
		ShowWindow(getHoverButtonHwnd(hbClock), SW_HIDE);
		break;
	case SCREEN_CONTACT_INFO:
	case SCREEN_CONTACT_ADD:
	case SCREEN_CONTACT_EDIT:
		if (screen == SCREEN_CONTACT_INFO)
		{
			setHoverButtonStateImages(hbContactInfoTitleBg, IDB_CONTACT_INFO_WND_APP_NAME, IDB_CONTACT_INFO_WND_APP_NAME);
			setHoverButtonStateImages(hbEditSaveContact, IDB_CONTACT_INFO_EDIT_CONTACT, IDB_CONTACT_INFO_EDIT_CONTACT);
		}
		else if (screen == SCREEN_CONTACT_ADD)
		{
			setHoverButtonStateImages(hbContactInfoTitleBg, IDB_CONTACT_ADD_WND_APP_NAME, IDB_CONTACT_ADD_WND_APP_NAME);
			setHoverButtonStateImages(hbEditSaveContact, IDB_CONTACT_EDIT_DONE, IDB_CONTACT_EDIT_DONE);
		}
		else if (screen == SCREEN_CONTACT_EDIT)
		{
			setHoverButtonStateImages(hbContactInfoTitleBg, IDB_CONTACT_EDIT_WND_APP_NAME, IDB_CONTACT_EDIT_WND_APP_NAME);
			setHoverButtonStateImages(hbEditSaveContact, IDB_CONTACT_EDIT_DONE, IDB_CONTACT_EDIT_DONE);
		}
		AnimateWindow(hwndContainerContactDetails, 400, AW_ACTIVATE | AW_HOR_POSITIVE);
		ShowWindow(getHoverButtonHwnd(hbMainCenterPic), SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbMainUnderDateBg), SW_HIDE);
		ShowWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), SW_HIDE);
		ShowWindow(hwndContainerMainButtons, SW_HIDE);
		ShowWindow(hwndContainerMiscButtons, SW_SHOW);
		ShowWindow(hwndContainerContacts, SW_HIDE);
		ShowWindow(getHoverButtonHwnd(hbClock), SW_HIDE);
		break;
	case SCREEN_CLOCK:
		ShowWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), SW_HIDE);
		ShowWindow(hwndContainerMiscButtons, SW_HIDE);
		ShowWindow(hwndContainerContacts, SW_HIDE);
		ShowWindow(hwndContainerContactDetails, SW_HIDE);
		AnimateWindow(getHoverButtonHwnd(hbMainUnderDateBg), 100, AW_HIDE | AW_VER_POSITIVE);
		AnimateWindow(getHoverButtonHwnd(hbMainCenterPic), 100, AW_HIDE | AW_VER_POSITIVE);
		AnimateWindow(hwndContainerMainButtons, 100, AW_HIDE | AW_VER_POSITIVE);
		AnimateWindow(getHoverButtonHwnd(hbClock), 200, AW_ACTIVATE | AW_CENTER);
		break;
	}
	screenMode = screen;
}

void enableChildContainers(BOOL value)
{	
	EnableWindow(getHoverButtonHwnd(hbMainCenterPic), value);
	EnableWindow(getHoverButtonHwnd(hbMainUnderDateBg), value);
	EnableWindow(GetParent(getHoverButtonHwnd(hbContainerCall)), value);
	EnableWindow(hwndContainerMainButtons, value);
	EnableWindow(hwndContainerMiscButtons, value);
	EnableWindow(hwndContainerContacts, value);
	InvalidateRect(GetParent(hwndConfirmDialog), NULL, FALSE);
}

void createGUI(HWND hWnd, HINSTANCE hInstance)
{
	HoverButton *tempBtn;
	HWND		temphWnd;

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
	hbClock = createHoverButton(hWnd, hInstance, 67, 156, 320, 460, 0, IDB_CLOCK_BG, IDB_CLOCK_BG, NULL);
	lockHoverButtonImage(hbClock, TRUE);
	setHoverButtonTextColor(hbClock, RGB(255, 255, 255));
	setHoverButtonFont(hbClock, TEXT("Atomic Clock Radio"), 40);
	ShowWindow(getHoverButtonHwnd(hbClock), SW_HIDE);
	hwndContainerMainButtons = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 67, 524, 320, 92, hWnd, NULL, hInstance, NULL);
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
	hbMainActionBtn[1] = createHoverButton(hwndContainerMainButtons, hInstance, x, y, width, height, BUTTON_ID_CONTACTS, IDB_MAIN_WND_CONTACT_ON, IDB_MAIN_WND_CONTACT_OFF, NULL);
	x += width;
	hbMainActionBtn[2] = createHoverButton(hwndContainerMainButtons, hInstance, x, y, width+1, height, BUTTON_ID_INFO, IDB_MAIN_WND_INFO_ON, IDB_MAIN_WND_INFO_OFF, NULL);
	x += width+1;
	hbMainActionBtn[3] = createHoverButton(hwndContainerMainButtons, hInstance, x, y, width, height, BUTTON_ID_BIN, IDB_MAIN_WND_BIN_EMPTY_ON, IDB_MAIN_WND_BIN_EMPTY_OFF, NULL);
	
	hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CONTACT_WND_BUTTON_BG));
	hwndContainerMiscButtons = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD | WS_CLIPCHILDREN/* | WS_VISIBLE*/ | SS_BITMAP, 67, 550, 320, 66, hWnd, NULL, hInstance, NULL);
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

	temphWnd = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD | WS_CLIPCHILDREN, 67, 253, 320, 271, hWnd, NULL, hInstance, NULL);
	SetWindowLong(temphWnd, GWL_WNDPROC, (LONG_PTR)ContainerProc);
	hbContainerCall = createHoverButton(temphWnd, hInst, 0, 0, 320, 271, 0, IDB_MAIN_BG_CALL, IDB_MAIN_BG_CALL, NULL);
	lockHoverButtonImage(hbContainerCall, TRUE);
	setHoverButtonTextColor(hbContainerCall, RGB(255, 255, 255));
	tempBtn = createHoverButton(getHoverButtonHwnd(hbContainerCall), hInst, 97, 204, 127, 42, BUTTON_ID_END_CALL, IDB_END_CALL_ON, IDB_END_CALL_OFF, NULL);


	hwndContainerContacts = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD | WS_CLIPCHILDREN/* | WS_VISIBLE*/, 67, 156, 320, 394, hWnd, NULL, hInstance, NULL);
	SetWindowLong(hwndContainerContacts, GWL_WNDPROC, (LONG_PTR)ContainerProc);
	// Create an Custom-Drawn list view control - see ListView.cpp for details.
	hLV = createListView(hwndContainerContacts, hInst, 0, 88, 320, 306);
	hbContactsTitleBg = createHoverButton(hwndContainerContacts, hInstance, 0, 0, 320, 44, 0, IDB_CONTACT_WND_APP_NAME, IDB_CONTACT_WND_APP_NAME, NULL);
	lockHoverButtonImage(hbContactsTitleBg, TRUE);
	hbAddContactEmpty = createHoverButton(getHoverButtonHwnd(hbContactsTitleBg), hInstance, 254, 7, 50, 30, BUTTON_ID_ADD_CONTACT_EMPTY, IDB_CONTACT_ADD, IDB_CONTACT_ADD, NULL);
	hbRecoverContact = createHoverButton(getHoverButtonHwnd(hbContactsTitleBg), hInstance, 13, 7, 91, 30, BUTTON_ID_REOVER_CONTACT, IDB_TRASH_BUTTON_RECOVER, IDB_TRASH_BUTTON_RECOVER, NULL);
	hbContactsSearchBg = createHoverButton(hwndContainerContacts, hInstance, 0, 44, 320, 44, 0, IDB_CONTACT_WND_SEARCH, IDB_CONTACT_WND_SEARCH, NULL);
	lockHoverButtonImage(hbContactsSearchBg, TRUE);
	EnableWindow(hbContactsSearchBg->hButton, FALSE);
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
		hwndConfirmDialog = CreateWindowEx(0, TEXT("static"), NULL,  WS_CHILD | WS_CLIPCHILDREN | SS_BITMAP,
			ifoneScreenRect.left + (ifoneScreenRect.right - ifoneScreenRect.left - 277) / 2,
			ifoneScreenRect.top + (ifoneScreenRect.bottom - ifoneScreenRect.top - 103) / 2,
			277, 103, hWnd, NULL, hInstance, NULL);
	}
	SetWindowLong(hwndConfirmDialog, GWL_WNDPROC, (LONG_PTR)ContainerProc);
	SendMessage(hwndConfirmDialog, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
	hbYes = createHoverButton(hwndConfirmDialog, hInstance, 8, 53, 128, 43, BUTTON_ID_YES, IDB_ALERT_YES_ON, IDB_ALERT_YES_OFF, NULL);
	hbNo = createHoverButton(hwndConfirmDialog, hInstance, 143, 53, 128, 43, BUTTON_ID_NO, IDB_ALERT_NO_ON, IDB_ALERT_NO_OFF, NULL);


	//createClock(hWnd, hInstance, 450, 0, 320, 480, 0, IDB_CLOCK_WND_BG);

	{
		// create contact details 
		// reference http://www.codeproject.com/KB/dialog/scroll_dialog.aspx
		hwndContainerContactDetails = CreateWindowEx(0, TEXT("static"), NULL, WS_CHILD | WS_CLIPCHILDREN, 67, 156, 320, 394, hWnd, NULL, hInstance, NULL);
		SetWindowLong(hwndContainerContactDetails, GWL_WNDPROC, (LONG_PTR)ContainerProc);
		hbContactInfoTitleBg = createHoverButton(hwndContainerContactDetails, hInstance, 0, 0, 320, 44, 0, IDB_CONTACT_INFO_WND_APP_NAME, IDB_CONTACT_INFO_WND_APP_NAME, NULL);
		lockHoverButtonImage(hbContactInfoTitleBg, TRUE);

		x = 0;
		y = 0;
		width = 320 - GetSystemMetrics(SM_CXVSCROLL);
		height = 44;

		hbAllContacts = createHoverButton(getHoverButtonHwnd(hbContactInfoTitleBg), hInstance, 13, 7, 91, 30, BUTTON_ID_ALL_CONTACTS, IDB_CONTACT_INFO_ALL_CONTACTS, IDB_CONTACT_INFO_ALL_CONTACTS, NULL);
		hbEditSaveContact = createHoverButton(getHoverButtonHwnd(hbContactInfoTitleBg), hInstance, 254, 7, 50, 30, BUTTON_ID_EDIT_SAVE_CONTACT, IDB_CONTACT_INFO_EDIT_CONTACT, IDB_CONTACT_INFO_EDIT_CONTACT, NULL);
		// Needed width is 320 pixels - width of vertical scrollbar.
		// Needed height is 64 pixels (44 button + 20 label) * 11 + 350 % 64 (addition of this fraction gives a nice, uniform effect to the list)
		hwndScrollContainer = createScrollContainer(hwndContainerContactDetails, hInstance, 0, 0, 44, 320, 350, width, height + 20, 
			width, ((height + 20) * 11) + (350 % 64), 0, IDB_CONTACT_INFO_WND_BG);

		hlblContactInfo[0] = CreateWindowEx(0, TEXT("static"), TEXT("First name"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		defTransparentLabelProc = (WNDPROC)SetWindowLong(hlblContactInfo[0], GWL_WNDPROC, (LONG_PTR)TransparentLabelProc);
		y += 20;
		ebContactInfo[0] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_LAST_NAME, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[1] = CreateWindowEx(0, TEXT("static"), TEXT("Last name"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[1] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_FIRST_NAME, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[2] = CreateWindowEx(0, TEXT("static"), TEXT("Phone number"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[2] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_PHONE, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[3] = CreateWindowEx(0, TEXT("static"), TEXT("Skype handle"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[3] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_SKYPE, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[4] = CreateWindowEx(0, TEXT("static"), TEXT("E-mail"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[4] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_EMAIL, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[5] = CreateWindowEx(0, TEXT("static"), TEXT("Web site"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[5] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_WEB, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[6] = CreateWindowEx(0, TEXT("static"), TEXT("Age"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[6] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_AGE, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[7] = CreateWindowEx(0, TEXT("static"), TEXT("Country"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[7] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_ADDRESS_CNT, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[8] = CreateWindowEx(0, TEXT("static"), TEXT("City"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[8] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_ADDRESS_CTY, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[9] = CreateWindowEx(0, TEXT("static"), TEXT("Street"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[9] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_ADDRESS_STR, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		hlblContactInfo[10] = CreateWindowEx(0, TEXT("static"), TEXT("Number"), WS_CHILD | WS_VISIBLE, 0, y, 320, 20, hwndScrollContainer, NULL, hInstance, NULL);
		y += 20;
		ebContactInfo[10] = createEditButton(hwndScrollContainer, hInstance, x, y, width, height, INFO_ID_ADDRESS_NUM, IDB_CONTACT_WND_NAME_BG_ON, IDB_CONTACT_WND_NAME_BG_OFF, NULL);
		y += height;
		for (y = 0; y < 11; y++)
		{
			setEditButtonImageStretch(ebContactInfo[y], TRUE);
			lockEditButton(ebContactInfo[y], TRUE);
		}
		hbEmail = createHoverButton(hwndScrollContainer, hInstance, 0, 0, 42, 42, BUTTON_ID_SEND_EMAIL, IDB_CONTACT_DTL_EMAIL_ON, IDB_CONTACT_DTL_EMAIL_OFF, NULL);
		hbGoogleMap = createHoverButton(hwndScrollContainer, hInstance, 0, 0, 42, 42, BUTTON_ID_SEE_ON_MAP, IDB_CONTACT_DTL_MAP_ON, IDB_CONTACT_DTL_MAP_OFF, NULL);
		hbSkypeHandle = createHoverButton(hwndScrollContainer, hInstance, 0, 0, 42, 42, BUTTON_ID_SKYPE_HANDLE, IDB_CONTACT_DTL_SKYPE_ON, IDB_CONTACT_DTL_SKYPE_OFF, NULL);
		lockHoverButtonImage(hbSkypeHandle, TRUE);
		hbWebsite = createHoverButton(hwndScrollContainer, hInstance, 0, 0, 42, 42, BUTTON_ID_GO_TO_WEBSITE, IDB_CONTACT_DTL_WWW_ON, IDB_CONTACT_DTL_WWW_OFF, NULL);
	}

	//setEditButtonFont(ebContactInfo[0], TEXT("Arial"), 10);	
	//hwndAddressContainer = CreateWindowEx(WS_EX_TOPMOST, TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP, 0, 44, 320, 264, hwndContainerContactDetails, NULL, hInstance, NULL);
}