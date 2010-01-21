#include "StdAfx.h"
#include "resource.h"
#include "PromptBox.h"
#include "HoverButton.h"
#include "Miscellaneous.h"

INT_PTR CALLBACK PromptBoxProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE hInst;

INT_PTR promptBox(HWND hwndOwner, LPCTSTR lpText, PbType pbType)
{
	const void *paramArray[2];
	paramArray[0] = lpText;
	paramArray[1] = &pbType;
	hInst = GetModuleHandle(NULL);

	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROMPTBOX), hwndOwner, PromptBoxProc, (LPARAM)paramArray);
}

// Message handler for about box.
INT_PTR CALLBACK PromptBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			const void **paramArray = (void**)lParam;
			const LPCTSTR lpText = paramArray[0];
			const PbType pbType = *(PbType*)paramArray[1];
			HoverButton *hbOk, *hbCancel = NULL;
			POINT pt = { 0, 0 };

			switch (pbType)
			{
			case PB_OKCANCEL:
				hbOk = createHoverButton(hDlg, hInst, 5, 55, 128, 43, IDOK, IDB_ALERT_YES_ON, IDB_ALERT_YES_OFF, NULL);
				setHoverButtonText(hbOk, TEXT("OK"));
				hbCancel = createHoverButton(hDlg, hInst, 144, 55, 128, 43, IDCANCEL, IDB_ALERT_NO_ON, IDB_ALERT_NO_OFF, NULL);
				setHoverButtonText(hbCancel, TEXT("Cancel"));
			case PB_YESNO:
				hbOk = createHoverButton(hDlg, hInst, 5, 55, 128, 43, IDYES, IDB_ALERT_YES_ON, IDB_ALERT_YES_OFF, NULL);
				setHoverButtonText(hbOk, TEXT("Yes"));
				hbCancel = createHoverButton(hDlg, hInst, 144, 55, 128, 43, IDNO, IDB_ALERT_NO_ON, IDB_ALERT_NO_OFF, NULL);
				setHoverButtonText(hbCancel, TEXT("No"));
				break;
			case PB_OK:
				hbOk = createHoverButton(hDlg, hInst, 75, 55, 128, 43, IDOK, IDB_ALERT_YES_ON, IDB_ALERT_YES_OFF, NULL);
				setHoverButtonText(hbOk, TEXT("OK"));
				break;
			default:
				hbOk = createHoverButton(hDlg, hInst, 75, 55, 128, 43, IDOK, IDB_ALERT_YES_OFF, IDB_ALERT_NO_OFF, NULL);
				setHoverButtonText(hbOk, TEXT("OK"));
				break;
			}
			setHoverButtonFont(hbOk, TEXT("Arial"), 12, TRUE);
			setHoverButtonTextColor(hbOk, RGB(255, 255, 255));
			if (hbCancel)
			{
				setHoverButtonFont(hbCancel, TEXT("Arial"), 12, TRUE);
				setHoverButtonTextColor(hbCancel, RGB(255, 255, 255));
			}
			ClientToScreen(GetParent(hDlg), &pt);
			pt.x = pt.x + 67 + 22;
			pt.y = pt.y + 136 + 189;
			MoveWindow(hDlg, pt.x, pt.y, 277, 103, TRUE);
			SetWindowLong(hDlg, GWL_EXSTYLE, GetWindowLong(hDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(hDlg, 0, 200, LWA_ALPHA);

			MoveWindow(GetDlgItem(hDlg, IDC_PROMPTTEXT), 5, 5, 267, 47, TRUE);
			SetWindowText(GetDlgItem(hDlg, IDC_PROMPTTEXT), lpText);
		}
	case WM_COMMAND:
		{
			int wmId    = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			if (wmEvent == HOVER_BUTTON_LMOUSE_UP)
			{
				EndDialog(hDlg, wmId);
				return (INT_PTR)TRUE;
			}
		}
		break;
	case WM_CTLCOLORSTATIC:
		SetBkMode((HDC)wParam, TRANSPARENT);
		SetTextColor((HDC)wParam, 0xFFFFFF); // White color
		return (INT_PTR)(HBRUSH)GetStockObject(NULL_BRUSH);
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hDlg, &ps);
			setImageToDc(hInst, &ps.rcPaint, &ps.rcPaint, hdc, IDB_ALERT_BG);
			EndPaint(hDlg, &ps);
		}
		break;
	}
	return (INT_PTR)FALSE;
}
