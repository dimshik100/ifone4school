#include "stdafx.h"
#include "SkypeAPI.h"
#include "DynamicListC.h"
#include <stdio.h>

#define CLASS_WND_NAME TEXT("iFoneBookSkypeApiWindow")
#define CALL_OBJECT_MUTEX TEXT("SkypeCallObjectMutex")

#define TIMER_ID		6000
#define PING_TIMER_ID	TIMER_ID + 1

LPTSTR					getStringFromMessage(PCOPYDATASTRUCT copyData);
LPSTR					getAnsiStringFromString(LPTSTR string);
ATOM					registerSkypeApiWindowClass(HINSTANCE hInstance);
LRESULT					sendSkypeMessage(LPTSTR message);
LRESULT CALLBACK		SkypeApiWndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI			SkypeCallGetterThreadProc(__in  LPVOID lpParameter);
DWORD WINAPI			SkypeQueueManagerThreadProc(__in  LPVOID lpParameter);
VOID CALLBACK			PingTimerProc(HWND, UINT, UINT_PTR, DWORD);

UINT							msgIdApiAttach = 0;
UINT							msgIdApiDiscover = 0;
HWND							skypeApiWindowHandle = NULL;
HWND							hiddenWindowHandle = NULL;
HINSTANCE						hInst = NULL;
SkypeCallStatusCallback			skypeCallStatusCallback = NULL;
SkypeConnectionStatusCallback	skypeConnectionStatusCallback = NULL;

int						queueManagerActive = 0;
int						skypePingStatus = 1;
DynamicListC			callObjectList = NULL;
DynamicListC			callObjectQueue = NULL;
HANDLE					hMutex = NULL, queueManagerHandle = NULL;

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
	hInst = hInstance;
	if (!msgIdApiAttach || !msgIdApiDiscover || !hiddenWindowHandle)
		if (!registerSkypeApi(hInstance))
			return FALSE;
	if (!callObjectList)
		listInit(&callObjectList);
	if (!callObjectQueue)
		listInit(&callObjectQueue);
	if (!hMutex)
	{
		hMutex = CreateMutex(NULL, FALSE, CALL_OBJECT_MUTEX);
		ReleaseMutex(hMutex);
	}
	if (!queueManagerActive)
	{
		queueManagerActive = 1;
		queueManagerHandle = CreateThread(NULL, 0, SkypeQueueManagerThreadProc, NULL, 0, NULL);
	}
	return SendMessage(HWND_BROADCAST, msgIdApiDiscover, (WPARAM)hiddenWindowHandle, 0);
}

void disconnectSkype(HINSTANCE hInstance)
{
	// In case disconnectSkype was called already.
	if (!hMutex)
		return;

	queueManagerActive = 0;
	KillTimer(hiddenWindowHandle, PING_TIMER_ID);
	skypeApiWindowHandle = NULL;
	DestroyWindow(hiddenWindowHandle);
	hiddenWindowHandle = NULL;
	UnregisterClass(CLASS_WND_NAME, hInstance);
	WaitForSingleObject(queueManagerHandle, INFINITE);
	CloseHandle(queueManagerHandle);
	queueManagerHandle = NULL;
	CloseHandle(hMutex);
	hMutex = NULL;
	if (callObjectList)
	{
		SkypeCallObject *skypeCallObject;
		listInit(&callObjectList);
		for (listSelectFirst(callObjectList); listSelectCurrent(callObjectList); listSelectNext(callObjectList, NULL))
		{
			listGetValue(callObjectList, NULL, &skypeCallObject);
			if (skypeCallObject->partnerDisplayName)
				free(skypeCallObject->partnerDisplayName);
			if (skypeCallObject->partnerHandle)
				free(skypeCallObject->partnerHandle);
		}
		listFree(&callObjectList);
	}
	if (!callObjectQueue)
		listFree(&callObjectQueue);
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
			skypePingStatus = 1;
			SetTimer(hiddenWindowHandle, PING_TIMER_ID, 5000, PingTimerProc);
			skypeConnectionStatusCallback(ATTACH_SUCCESS);
			break;
		case ATTACH_PENDING:
			skypeConnectionStatusCallback(ATTACH_PENDING);
			break;
		case ATTACH_REFUSED:
			skypeConnectionStatusCallback(ATTACH_REFUSED);
			break;
		case ATTACH_CONNECTION_LOST:
		case ATTACH_NOT_AVAILABLE:	
			skypeApiWindowHandle = NULL;
			KillTimer(hiddenWindowHandle, PING_TIMER_ID);
			skypeConnectionStatusCallback((SkypeApiInitStatus)lParam);
			break;
		case ATTACH_AVAILABLE:
			SendMessage(HWND_BROADCAST, msgIdApiDiscover, (WPARAM)hiddenWindowHandle, 0);
			skypeConnectionStatusCallback(ATTACH_AVAILABLE);
			break;
		}
	}

	return ret;
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

void setSkypeCallStatusCallback(SkypeCallStatusCallback newSkypeCallStatusCallback)
{
	skypeCallStatusCallback = newSkypeCallStatusCallback;
}

void setSkypeConnectionStatusCallback(SkypeConnectionStatusCallback newSkypeConnectionStatusCallback)
{
	skypeConnectionStatusCallback = newSkypeConnectionStatusCallback;
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
			int commandId = 0;

			token = _tcstok_s(string, seps, &next_token);
			if (token && token[0] == TEXT('#'))
			{
				commandId = _tstoi(token+1);
				token = _tcstok_s(NULL, seps, &next_token);
			}
			if (!_tcscmp(token, TEXT("CALL")))
			{
				SkypeCallObject *callObject = (SkypeCallObject*)calloc(1, sizeof(SkypeCallObject));
				callObject->object = OBJECT_CALL;
				callObject->commandId = commandId;
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
						callObject->type = CALLTYPE_INCOMING_PSTN;
					else if (!_tcscmp(token, TEXT("INCOMING_P2P")))
						callObject->type = CALLTYPE_INCOMING_P2P;
					else if (!_tcscmp(token, TEXT("OUTGOING_PSTN")))
						callObject->type = CALLTYPE_OUTGOING_PSTN;
					else if (!_tcscmp(token, TEXT("OUTGOING_P2P")))
						callObject->type = CALLTYPE_OUTGOING_P2P;
				}
				else if (!_tcscmp(token, TEXT("PARTNER_HANDLE")))
				{
					callObject->property = CALLPROPERTY_PARTNER_HANDLE;
					callObject->partnerHandle = _tcsdup(next_token);
				}
				else if (!_tcscmp(token, TEXT("PARTNER_DISPNAME")))
				{
					callObject->property = CALLPROPERTY_PARTNER_DISPNAME;
					callObject->partnerDisplayName = _tcsdup(next_token);
				}

				*skypeObject = (SkypeObject*)callObject;
			}
			else if (!_tcscmp(token, TEXT("PONG")))
			{
				*skypeObject = (SkypeObject*)calloc(1, sizeof(SkypeObject));
				(*skypeObject)->commandId = commandId;
				(*skypeObject)->object = OBJECT_PONG;
			}
			// Return true confirm we processed this message
			ret = TRUE;
			free(string);
		}
	}

	return ret;
}

// Depending on project settings, must handle strings dfferently between UNICODE and ANSI
#ifdef _UNICODE

LPTSTR getStringFromMessage(PCOPYDATASTRUCT copyData)
{
	LPTSTR string = (LPTSTR)malloc((sizeof(TCHAR)*(copyData->cbData + 1)));
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)copyData->lpData, (int)copyData->cbData, string, (int)copyData->cbData);
	return string;
}

LPSTR getAnsiStringFromString(LPTSTR string)
{
	size_t strLen = _tcslen(string) + 1;
	LPSTR ansiString = (LPSTR)malloc((sizeof(CHAR)*strLen));
	sprintf_s(ansiString, strLen, "%S", string);
	return ansiString;
}

#else

LPTSTR getStringFromMessage(PCOPYDATASTRUCT copyData)
{
	LPTSTR string;
	string = (LPTSTR)malloc((sizeof(TCHAR)*(copyData->cbData + 1)));
	_tcscpy_s(string, copyData->cbData, copyData->lpData);
	return string;
}

LPCSTR getAnsiStringFromString(LPTSTR string)
{
	size_t strLen = _tcslen(string) + 1;
	LPSTR ansiString = (LPSTR)malloc((sizeof(CHAR)*strLen));
	sprintf_s(ansiString, strLen, "%s", string);
	return ansiString;
}

#endif

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

LRESULT sendSkypeMessage(LPTSTR message)
{
	LRESULT ret = FALSE;

	if (skypeApiWindowHandle)
	{
		COPYDATASTRUCT copyData = {0};
		LPSTR ansiString = getAnsiStringFromString(message);

		if (ansiString)
		{
			copyData.lpData = ansiString;
			copyData.cbData = strlen(ansiString) + 1;
			ret = SendMessage(skypeApiWindowHandle, WM_COPYDATA, (WPARAM)hiddenWindowHandle, (LPARAM)&copyData);
			free(ansiString);
		}
	}

	return ret;
}

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
				case OBJECT_PONG:
					skypePingStatus = 1;
					break;
				case OBJECT_CALL:
					if (!WaitForSingleObject(hMutex, 100))
					{
						SkypeCallObject *translatedCallObject = (SkypeCallObject*)skypeObject,  *callObject = NULL, *queuedCall;
						if (translatedCallObject->commandId == 0)
							listInsertAfterEnd(callObjectQueue, &translatedCallObject);
						for (listSelectFirst(callObjectList); listSelectCurrent(callObjectList); listSelectNext(callObjectList, NULL))
						{
							listGetValue(callObjectList, NULL, &callObject);
							if (callObject && callObject->callId == translatedCallObject->callId)
								break;
							else
								callObject = NULL;
						}
						if (callObject)
						{
							switch(translatedCallObject->property)
							{
							case CALLPROPERTY_TYPE:
								callObject->type = translatedCallObject->type;
								break;
							case CALLPROPERTY_DURATION:
								callObject->duration = translatedCallObject->duration;
								break;
							case CALLPROPERTY_STATUS:
								callObject->status = translatedCallObject->status;
								break;
							case CALLPROPERTY_PARTNER_HANDLE:
								callObject->partnerHandle = translatedCallObject->partnerHandle;
								break;
							case CALLPROPERTY_PARTNER_DISPNAME:
								callObject->partnerDisplayName = translatedCallObject->partnerDisplayName;
								break;
							}
							for (listSelectFirst(callObjectQueue); listSelectCurrent(callObjectQueue); listSelectNext(callObjectQueue, NULL))
							{
								listGetValue(callObjectQueue, NULL, &queuedCall);
								if (queuedCall && queuedCall->callId == callObject->callId)
								{
									queuedCall->duration = callObject->duration;									
									queuedCall->partnerHandle = callObject->partnerHandle;
									queuedCall->partnerDisplayName = callObject->partnerDisplayName;
									if (queuedCall->type == CALLTYPE_UNKNOWN)
										queuedCall->type = callObject->type;
									if (queuedCall->status == CALLSTATUS_UNKNOWN)
										queuedCall->status = callObject->status;
								}
							}
							ReleaseMutex(hMutex);
						}
						else
						{
							SkypeCallObject *newCallObject = (SkypeCallObject*)malloc(sizeof(SkypeCallObject));
							*newCallObject = *translatedCallObject;
							listInsertBeforeStart(callObjectList, &newCallObject);
							ReleaseMutex(hMutex);
							Sleep(0);
							CloseHandle(CreateThread(NULL, 0, SkypeCallGetterThreadProc, translatedCallObject, 0, NULL));
						}
					}
					break;
				}
			}
			return TRUE;
		}
	}
	else if (processAttachmentMessage(message, wParam, lParam))
		return TRUE;

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void call(LPTSTR name)
{
	if (name)
	{
		TCHAR str[256];
		_stprintf_s(str, 256, TEXT("CALL %s"), name);
		sendSkypeMessage(str);
	}
}

void hangup(int callId)
{
	TCHAR str[256];
	_stprintf_s(str, 256, TEXT("SET CALL %d STATUS FINISHED"), callId);
	sendSkypeMessage(str);
}

DWORD WINAPI SkypeCallGetterThreadProc(__in  LPVOID lpParameter)
{
	COPYDATASTRUCT copyData = {0};
	static int commandId = 1;
	char str[256];

	SkypeCallObject *callObject = (SkypeCallObject*)lpParameter;
	if (!WaitForSingleObject(hMutex, 100))
	{
		copyData.lpData = str;
		copyData.cbData = strlen(str)+1;
		sprintf_s(str, 256, "#%d GET CALL %d TYPE", commandId++, callObject->callId);		
		SendMessage(skypeApiWindowHandle, WM_COPYDATA, (WPARAM)hiddenWindowHandle, (LPARAM)&copyData);
		sprintf_s(str, 256, "#%d GET CALL %d DURATION", commandId++, callObject->callId);		
		SendMessage(skypeApiWindowHandle, WM_COPYDATA, (WPARAM)hiddenWindowHandle, (LPARAM)&copyData);
		sprintf_s(str, 256, "#%d GET CALL %d STATUS", commandId++, callObject->callId);		
		SendMessage(skypeApiWindowHandle, WM_COPYDATA, (WPARAM)hiddenWindowHandle, (LPARAM)&copyData);
		sprintf_s(str, 256, "#%d GET CALL %d PARTNER_HANDLE", commandId++, callObject->callId);		
		SendMessage(skypeApiWindowHandle, WM_COPYDATA, (WPARAM)hiddenWindowHandle, (LPARAM)&copyData);
		sprintf_s(str, 256, "#%d GET CALL %d PARTNER_DISPNAME", commandId++, callObject->callId);		
		SendMessage(skypeApiWindowHandle, WM_COPYDATA, (WPARAM)hiddenWindowHandle, (LPARAM)&copyData);
		ReleaseMutex(hMutex);
	}
	return 0;
}

DWORD WINAPI SkypeQueueManagerThreadProc(__in  LPVOID lpParameter)
{
	int timerCnt;
	SkypeObject *skypeObject;
	UNREFERENCED_PARAMETER(lpParameter);

	while (queueManagerActive)
	{		
		if (listGetListCount(callObjectQueue) && !WaitForSingleObject(hMutex, 100))
		{
			listSelectFirst(callObjectQueue);
			listGetValue(callObjectQueue, NULL, &skypeObject);
			ReleaseMutex(hMutex);
			timerCnt = 0;
			switch(skypeObject->object)
			{
			case OBJECT_CALL:
				{
					SkypeCallObject *callObject = (SkypeCallObject*)skypeObject;
					for ( ; callObject; )
					{
						if (!WaitForSingleObject(hMutex, 100))
						{
							if (callObject->type != CALLTYPE_UNKNOWN && callObject->status != CALLSTATUS_UNKNOWN && 
								callObject->partnerHandle && callObject->partnerDisplayName)
							{
								skypeCallStatusCallback(callObject);
								break;
							}
							else if (timerCnt >= 3000)
								break;
							ReleaseMutex(hMutex);
						}
						timerCnt += 50;
						Sleep(50);
					}
					listDeleteNode(callObjectQueue, listSelectFirst(callObjectQueue));
				}
				break;
			}
			ReleaseMutex(hMutex);
		}
		Sleep(10);
	}
	return 0;
}

VOID CALLBACK		PingTimerProc(HWND hWnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
	UNREFERENCED_PARAMETER(hWnd), UNREFERENCED_PARAMETER(message), UNREFERENCED_PARAMETER(idEvent), UNREFERENCED_PARAMETER(dwTime);

	if (skypeApiWindowHandle)
	{
		if (skypePingStatus)
		{
			sendSkypeMessage(TEXT("PING"));
			skypeConnectionStatusCallback(ATTACH_ACTIVE);
		}
		else
			processAttachmentMessage(msgIdApiAttach, 0, ATTACH_CONNECTION_LOST);

		skypePingStatus = 0;
	}
}