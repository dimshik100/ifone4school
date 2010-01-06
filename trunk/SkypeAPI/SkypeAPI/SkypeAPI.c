#include "stdafx.h"
#include "SkypeAPI.h"
#include <stdio.h>

#define CLASS_WND_NAME TEXT("iFoneBookSkypeApiWindow")
UINT msgIdApiAttach = 0; //RegisterWindowMessage(TEXT("SkypeControlAPIAttach"));
UINT msgIdApiDiscover = 0; //RegisterWindowMessage(TEXT("SkypeControlAPIDiscover"));
HWND skypeApiWindowHandle = NULL, hiddenWindowHandle = NULL;
SkypeCallbackFunction skypeCallbackFunction;

LPTSTR getStringFromMessage(PCOPYDATASTRUCT copyData);
ATOM registerSkypeApiWindowClass(HINSTANCE hInstance);
//HWND createHiddenWindow(HINSTANCE hInstance);
LRESULT CALLBACK	SkypeApiWndProc(HWND, UINT, WPARAM, LPARAM);


BOOL registerSkypeApi(HINSTANCE hInstance)
{
	msgIdApiAttach = RegisterWindowMessage(TEXT("SkypeControlAPIAttach"));
	msgIdApiDiscover = RegisterWindowMessage(TEXT("SkypeControlAPIDiscover"));
	registerSkypeApiWindowClass(hInstance);
	hiddenWindowHandle = CreateWindowEx(0, CLASS_WND_NAME, NULL, WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, 0, hInstance, 0);
	return (hiddenWindowHandle) ? TRUE : FALSE;
}

LRESULT connectSkype(HINSTANCE hInstance)
{
	if (!msgIdApiAttach || !msgIdApiDiscover || !hiddenWindowHandle)
		if (!registerSkypeApi(hInstance))
			return FALSE;
	return SendMessage(HWND_BROADCAST, msgIdApiDiscover, (WPARAM)hiddenWindowHandle, 0);
}

BOOL processAttachmentMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL ret = FALSE;

	if (message == msgIdApiAttach)
	{
		// Return 'true' as we process this message.
		ret = TRUE;
		switch(lParam)
		{
		case ATTACH_SUCCESS:
			skypeApiWindowHandle = (HWND)wParam;
			break;
		case ATTACH_PENDING:
			break;
		case ATTACH_REFUSED:
			break;
		case ATTACH_NOT_AVAILABLE:
			break;
		case ATTACH_AVAILABLE:
			break;
		}
	}

	return ret;
}

void disconnectSkype()
{
	skypeApiWindowHandle = NULL;
}

UINT getMsgIdApiAttach()
{
	return msgIdApiAttach;
}

UINT getMsgIdApiDiscover()
{
	return msgIdApiDiscover;
}

HWND getSkypeApiWindowHandle()
{
	return skypeApiWindowHandle;
}

void setSkypeApiCallback(SkypeCallbackFunction newSkypeCallbackFunction)
{
	skypeCallbackFunction = newSkypeCallbackFunction;
}

BOOL translateSkypeMessage(WPARAM wParam, LPARAM lParam, SkypeObject **skypeObject)
{
	BOOL ret = FALSE;
	*skypeObject = NULL;

	if ((HWND)wParam == skypeApiWindowHandle)
	{
		LPTSTR string = getStringFromMessage((PCOPYDATASTRUCT)lParam);
		if (string)
		{
			TCHAR seps[] = TEXT(" ");
			LPTSTR token, next_token;

			token = _tcstok_s(string, seps, &next_token);
			if (!_tcscmp(token, TEXT("CALL")))
			{
				SkypeCallObject *callObject = (SkypeCallObject*)calloc(1, sizeof(SkypeCallObject));
				callObject->object = OBJECT_CALL;
				token = _tcstok_s(NULL, seps, &next_token);

				callObject->callId = _tstoi(token);
				token = _tcstok_s(NULL, seps, &next_token);

				if (!_tcscmp(token, TEXT("STATUS")))
				{
					callObject->property = CALLPROPERTY_STATUS;
					token = _tcstok_s(NULL, seps, &next_token);
					if (!_tcscmp(token, TEXT("UNPLACED")))
						callObject->status = CALLSTATUS_UNPLACED;
					else if (!_tcscmp(token, TEXT("ROUTING")))
						callObject->status = CALLSTATUS_ROUTING;
					else if (!_tcscmp(token, TEXT("INPROGRESS")))
						callObject->status = CALLSTATUS_IN_PROGRESS;
					else if (!_tcscmp(token, TEXT("ONHOLD")))
						callObject->status = CALLSTATUS_ON_HOLD;
					else if (!_tcscmp(token, TEXT("FINISHED")))
						callObject->status = CALLSTATUS_FINISHED;
					else if (!_tcscmp(token, TEXT("EARLYMEDIA")))
						callObject->status = CALLSTATUS_EARLY_MEDIA;
					else if (!_tcscmp(token, TEXT("FAILED")))
						callObject->status = CALLSTATUS_FAILED;
					else if (!_tcscmp(token, TEXT("RINGING")))
						callObject->status = CALLSTATUS_RINGING;
					else if (!_tcscmp(token, TEXT("MISSED")))
						callObject->status = CALLSTATUS_MISSED;
					else if (!_tcscmp(token, TEXT("REFUSED")))
						callObject->status = CALLSTATUS_REFUSED;
					else if (!_tcscmp(token, TEXT("BUSY")))
						callObject->status = CALLSTATUS_BUSY;
					else if (!_tcscmp(token, TEXT("CANCELLED")))
						callObject->status = CALLSTATUS_CANCELLED;
					else if (!_tcscmp(token, TEXT("TRANSFERRING")))
						callObject->status = CALLSTATUS_TRANSFERRING;
					else if (!_tcscmp(token, TEXT("TRANSFERRED")))
						callObject->status = CALLSTATUS_TRANSFERRED;
					else if (!_tcscmp(token, TEXT("WAITING_REDIAL_COMMAND")))
						callObject->status = CALLSTATUS_WAITING_REDIAL_COMMAND;
					else if (!_tcscmp(token, TEXT("REDIAL_PENDING")))
						callObject->status = CALLSTATUS_REDIAL_PENDING;
					else if (!_tcscmp(token, TEXT("VM_BUFFERING_GREETING")))
						callObject->status = CALLSTATUS_VM_BUFFERING_GREETING;
					else if (!_tcscmp(token, TEXT("VM_PLAYING_GREETING")))
						callObject->status = CALLSTATUS_VM_PLAYING_GREETING;
					else if (!_tcscmp(token, TEXT("VM_UPLOADING")))
						callObject->status = CALLSTATUS_VM_UPLOADING;
					else if (!_tcscmp(token, TEXT("VM_SENT")))
						callObject->status = CALLSTATUS_VM_SENT;
					else if (!_tcscmp(token, TEXT("VM_FAILED")))
						callObject->status = CALLSTATUS_VM_FAILED;
					else if (!_tcscmp(token, TEXT("VM_CANCELLED")))
						callObject->status = CALLSTATUS_VM_CANCELLED;
					else if (!_tcscmp(token, TEXT("VM_RECORDING")))
						callObject->status = CALLSTATUS_VM_RECORDING;

				}
				else if (!_tcscmp(token, TEXT("DURATION")))
				{
					callObject->property = CALLPROPERTY_DURATION;
					token = _tcstok_s(NULL, seps, &next_token);
					callObject->duration = _tstoi(token);
				}
				else if (!_tcscmp(token, TEXT("TYPE")))
				{
					callObject->property = CALLPROPERTY_TYPE;
					token = _tcstok_s(NULL, seps, &next_token);
					if (!_tcscmp(token, TEXT("INCOMING_PSTN")))
						callObject->status = CALLTYPE_INCOMING_PSTN;
					else if (!_tcscmp(token, TEXT("INCOMING_P2P")))
						callObject->status = CALLTYPE_INCOMING_P2P;
					else if (!_tcscmp(token, TEXT("OUTGOING_PSTN")))
						callObject->status = CALLTYPE_OUTGOING_PSTN;
					else if (!_tcscmp(token, TEXT("OUTGOING_P2P")))
						callObject->status = CALLTYPE_OUTGOING_P2P;
				}

				*skypeObject = (SkypeObject*)callObject;
			}
			// Return true confirm we processed this message
			ret = TRUE;
			free(string);
		}
	}

	return ret;
}

LPTSTR getStringFromMessage(PCOPYDATASTRUCT copyData)
{
	LPTSTR string;

	string = (LPTSTR)malloc((sizeof(TCHAR)*copyData->cbData) + 1);
// Depending on project settings, copy (or convert if in Unicode) the command string to the temporary string.
#ifdef _UNICODE
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)copyData->lpData, (int)copyData->cbData, string, (int)copyData->cbData);
#else
	_tcscpy_s(string, copyData->cbData, copyData->lpData);
#endif
	
	return string;
}

ATOM registerSkypeApiWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= SkypeApiWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= CLASS_WND_NAME;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

//HWND createHiddenWindow(HINSTANCE hInstance)
//{
//	HWND hWnd;
//	hWnd = CreateWindowEx(0, CLASS_WND_NAME, NULL, WS_SYSMENU | WS_MINIMIZEBOX,
//		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, 0, hInstance, 0);
//
//}

SkypeObject *mainSkypeObject = NULL;
LRESULT CALLBACK SkypeApiWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COPYDATA)
	{
		static int objectGetting = 0;
		SkypeObject *skypeObject;
		if (translateSkypeMessage(wParam, lParam, &skypeObject))
		{			
			if (skypeObject)
			{
				switch (skypeObject->object)
				{
				case OBJECT_CALL:
					{
						SkypeCallObject *callObject = (SkypeCallObject*)skypeObject;
						callObject->object = OBJECT_CALL;
						if (!mainSkypeObject)
						{
							COPYDATASTRUCT copyData = {0};
							char a[256];
							LRESULT b;

							mainSkypeObject = (SkypeObject*)calloc(1, sizeof(SkypeCallObject));
							b = SendMessage(hWnd, message, wParam, lParam);
							b = b;

							sprintf(a, "GET CALL %d TYPE", ((SkypeCallObject*)skypeObject)->callId);
							
							copyData.dwData = 0;
							copyData.lpData = a;
							copyData.cbData = strlen(a);
							b = SendMessage(getSkypeApiWindowHandle(), WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&copyData);
							b = b;
						}
						else
						{
							((SkypeCallObject*)mainSkypeObject)->property = callObject->property;
							switch(callObject->property)
							{
							case CALLPROPERTY_TYPE:
								((SkypeCallObject*)mainSkypeObject)->type = callObject->type;
								break;
							case CALLPROPERTY_DURATION:
								((SkypeCallObject*)mainSkypeObject)->duration = callObject->duration;
								break;
							case CALLPROPERTY_STATUS:
								((SkypeCallObject*)mainSkypeObject)->status = callObject->status;
								break;
							}
						}
						if (!objectGetting)
							skypeCallbackFunction(mainSkypeObject);
					}
					break;
				}
				free(skypeObject);
			}
			return TRUE;
		}
	}
	else if (processAttachmentMessage(message, wParam, lParam))
		return TRUE;

	return DefWindowProc(hWnd, message, wParam, lParam);
}