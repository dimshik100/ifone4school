#include "stdafx.h"
#include "SkypeAPI.h"
#include "DynamicListC.h"
#include <stdio.h>
#include <tchar.h>
#include <malloc.h>

#define CLASS_WND_NAME TEXT("iFoneBookSkypeApiWindow")
#define CALL_OBJECT_MUTEX TEXT("SkypeCallObjectMutex")

#define TIMER_ID		16000
#define PING_TIMER_ID	TIMER_ID + 1

//#define _SKYPEAPIDEBUG
#ifdef _SKYPEAPIDEBUG
void WriteDebug(LPTSTR name, SkypeObject *skypeObject);
#endif

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

// Registers the Skype messages with Windows to allow communication between the two processes
// Registers and creates the hidden window to recieve messages from skype
SKYPEAPIDLL_API BOOL registerSkypeApi(HINSTANCE hInstance)
{
	msgIdApiAttach = RegisterWindowMessage(TEXT("SkypeControlAPIAttach"));
	msgIdApiDiscover = RegisterWindowMessage(TEXT("SkypeControlAPIDiscover"));
	registerSkypeApiWindowClass(hInstance);
	hiddenWindowHandle = CreateWindowEx(0, CLASS_WND_NAME, NULL, WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, 0, hInstance, 0);
	return (hiddenWindowHandle) ? TRUE : FALSE;
}

// Initiates the connection with Skype, registers and initiates all the required data, 
// Asks to connect with Skype if everything initiated correctly
SKYPEAPIDLL_API LRESULT connectSkype(HINSTANCE hInstance)
{
	hInst = hInstance;
	if (!msgIdApiAttach || !msgIdApiDiscover || !hiddenWindowHandle)
		if (!registerSkypeApi(hInstance))
			return FALSE;
	if (!callObjectList)
		listInit(&callObjectList);
	if (!callObjectQueue)
		listInit(&callObjectQueue);

	// Create a mutex object to synchronize the worker-threads with the main thread.
	if (!hMutex)
	{
		hMutex = CreateMutex(NULL, FALSE, CALL_OBJECT_MUTEX);
		ReleaseMutex(hMutex);
	}

	// Create and start the queue manager thread.
	if (!queueManagerActive)
	{
		queueManagerActive = 1;
		queueManagerHandle = CreateThread(NULL, 0, SkypeQueueManagerThreadProc, NULL, 0, NULL);
	}

	// Send a message to all running applications notifying that we're looking to connect with Skype
	return PostMessage(HWND_BROADCAST, msgIdApiDiscover, (WPARAM)hiddenWindowHandle, 0);
}

// Disconnects from Skype, releases all handles, frees all queues, terminates the worker threads
SKYPEAPIDLL_API void disconnectSkype(HINSTANCE hInstance)
{
	DWORD exitCode = 0;

	// In case disconnectSkype was already called.
	if (!hMutex)
		return;

	queueManagerActive = 0;
	KillTimer(hiddenWindowHandle, PING_TIMER_ID);
	skypeApiWindowHandle = NULL;
	DestroyWindow(hiddenWindowHandle);
	hiddenWindowHandle = NULL;
	UnregisterClass(CLASS_WND_NAME, hInstance);
	// If the queue manager thread has not finished yet, wait for it to be done.
	if (GetExitCodeThread(queueManagerHandle, &exitCode) && exitCode == STILL_ACTIVE)
		WaitForSingleObject(queueManagerHandle, INFINITE);
	CloseHandle(queueManagerHandle);
	queueManagerHandle = NULL;
	// Close the mutex object.
	CloseHandle(hMutex);
	hMutex = NULL;
	// Clear all the queues and call list.
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

// Processes the attachment messages Skype send to our application.
// Notifies subscriber about attachment status changes.
SKYPEAPIDLL_API BOOL processAttachmentMessage(UINT message, WPARAM wParam, LPARAM lParam)
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
			if (skypeConnectionStatusCallback)
				skypeConnectionStatusCallback(ATTACH_SUCCESS);
			break;
		case ATTACH_PENDING:
			if (skypeConnectionStatusCallback)
				skypeConnectionStatusCallback(ATTACH_PENDING);
			break;
		case ATTACH_REFUSED:
			if (skypeConnectionStatusCallback)
				skypeConnectionStatusCallback(ATTACH_REFUSED);
			break;
		case ATTACH_CONNECTION_LOST:
		case ATTACH_NOT_AVAILABLE:	
			skypeApiWindowHandle = NULL;
			KillTimer(hiddenWindowHandle, PING_TIMER_ID);
			if (skypeConnectionStatusCallback)
				skypeConnectionStatusCallback((SkypeApiInitStatus)lParam);
			break;
		case ATTACH_AVAILABLE:
			PostMessage(HWND_BROADCAST, msgIdApiDiscover, (WPARAM)hiddenWindowHandle, 0);
			if (skypeConnectionStatusCallback)
				skypeConnectionStatusCallback(ATTACH_AVAILABLE);
			break;
		}
	}

	return ret;
}

// Retrieves the message value for ApiAttach notifications from Skype
SKYPEAPIDLL_API UINT getMsgIdApiAttach()
{
	return msgIdApiAttach;
}

// Retrieves the message value for ApiDiscover notifications from Skype
SKYPEAPIDLL_API UINT getMsgIdApiDiscover()
{
	return msgIdApiDiscover;
}

// Returns the HWND of the Skype window
SKYPEAPIDLL_API HWND getSkypeApiWindowHandle()
{
	return skypeApiWindowHandle;
}

// Sets a callback function for call status notifications
SKYPEAPIDLL_API void setSkypeCallStatusCallback(SkypeCallStatusCallback newSkypeCallStatusCallback)
{
	skypeCallStatusCallback = newSkypeCallStatusCallback;
}

// Sets a callback function for connection status change notifications
SKYPEAPIDLL_API void setSkypeConnectionStatusCallback(SkypeConnectionStatusCallback newSkypeConnectionStatusCallback)
{
	skypeConnectionStatusCallback = newSkypeConnectionStatusCallback;
}

// Translates the string message received from Skype into Skype Objects (which are defined in SkypeAPI.h)
// In this application we only handle the Call object, as we don't need the rest of them
SKYPEAPIDLL_API BOOL translateSkypeMessage(WPARAM wParam, LPARAM lParam, SkypeObject **skypeObject)
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

			// If this is a numbered command we're getting a reply for, "write down" it's number
			token = _tcstok_s(string, seps, &next_token);
			if (token && token[0] == TEXT('#'))
			{
				commandId = _tstoi(token+1);
				token = _tcstok_s(NULL, seps, &next_token);
			}
			// Now we will determine the type of object Skype is notifying us about, 
			// and retrieve all it's data to fill the appropriate Skype-Object structure.

			// If this message concerns the Call object
			if (!_tcscmp(token, TEXT("CALL")))
			{
				SkypeCallObject *callObject = (SkypeCallObject*)calloc(1, sizeof(SkypeCallObject));
				callObject->object = OBJECT_CALL;
				callObject->commandId = commandId;
				token = _tcstok_s(NULL, seps, &next_token);

				// Save the Call ID
				callObject->callId = _tstoi(token);
				token = _tcstok_s(NULL, seps, &next_token);

				// Now determine what information we're receiving about a call.

				// Status changes
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
				// Duration changes of an on-going call
				else if (!_tcscmp(token, TEXT("DURATION")))
				{
					callObject->property = CALLPROPERTY_DURATION;
					token = _tcstok_s(NULL, seps, &next_token);
					callObject->duration = _tstoi(token);
				}
				// Notification about the type of call this is (incoming/outgoing)
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
				// Sends the Skype handle of the person we're talking to.
				else if (!_tcscmp(token, TEXT("PARTNER_HANDLE")))
				{
					callObject->property = CALLPROPERTY_PARTNER_HANDLE;
					callObject->partnerHandle = _tcsdup(next_token);
				}
				// Sends the Skype display name (in Skype) of the person we're talking to.
				else if (!_tcscmp(token, TEXT("PARTNER_DISPNAME")))
				{
					callObject->property = CALLPROPERTY_PARTNER_DISPNAME;
					callObject->partnerDisplayName = _tcsdup(next_token);
				}

				*skypeObject = (SkypeObject*)callObject;
			}
			// If we receive a Pong reply to our Ping.
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

// Makes an outgoing call via Skype to "name"
SKYPEAPIDLL_API void call(LPTSTR name)
{
	if (name)
	{
		TCHAR str[256];
		_stprintf_s(str, 256, TEXT("CALL %s"), name);
		sendSkypeMessage(str);
	}
}

// Hangs up a call by ID, this has no effect when a call has already ended
SKYPEAPIDLL_API void hangupCall(int callId)
{
	TCHAR str[256];
	_stprintf_s(str, 256, TEXT("SET CALL %d STATUS FINISHED"), callId);
	sendSkypeMessage(str);
}

// Answers (or brings back online from Hold) a call by ID
SKYPEAPIDLL_API void answerCall(int callId)
{
	TCHAR str[256];
	_stprintf_s(str, 256, TEXT("SET CALL %d STATUS INPROGRESS"), callId);
	sendSkypeMessage(str);
}

// Puts an active call on hold by ID
SKYPEAPIDLL_API void holdCall(int callId)
{
	TCHAR str[256];
	_stprintf_s(str, 256, TEXT("SET CALL %d STATUS ONHOLD"), callId);
	sendSkypeMessage(str);
}

// Transcodes/copies a string from the COPYDATASTRUCT supplied by Skype
// into a Unicode/Ansi (depending on project settings) string.
/* getStringFromMessage */

// If in Unicode mode, converts a unicode string into Ansi using the 'sprintf_s' function
// In Ansi mode, simply duplicates the string.
/* getAnsiStringFromString */


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

// Registers the hidden window's class.
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

// Sends a message to skype by wrapping it in the required datatype and using the correct format.
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

// Hidden window's procedure. Here we receive all the messages from skype, translate them
LRESULT CALLBACK SkypeApiWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COPYDATA)
	{
		static int objectGetting = 0;
		SkypeObject *skypeObject;
		// Check if this message is from Skype and translate it if it is.
		if (translateSkypeMessage(wParam, lParam, &skypeObject))
		{			
			if (skypeObject)
			{
				switch (skypeObject->object)
				{
				case OBJECT_PONG:
					// Simply reset time Ping-Pong flag to signal that connection with Skype is alive
					skypePingStatus = 1; 
					break;
				case OBJECT_CALL:
					if (!WaitForSingleObject(hMutex, 100))
					{
						SkypeCallObject *translatedCallObject = (SkypeCallObject*)skypeObject,  *callObject = NULL, *queuedCall;
						// If the message is not a reply to a request for information, add this object to the message queue.
						if (translatedCallObject->commandId == 0)
							listInsertAfterEnd(callObjectQueue, &translatedCallObject);
						// Try to see if this call already exists in our call list and pull it out if it exists.
						for (listSelectFirst(callObjectList); listSelectCurrent(callObjectList); listSelectNext(callObjectList, NULL))
						{
							listGetValue(callObjectList, NULL, &callObject);
							if (callObject && callObject->callId == translatedCallObject->callId)
								break;
							else
								callObject = NULL;
						}
						// If the call exists, then update it with the new infomation we have about it.
						// Then update the messages in our queue as well so they can be released.
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
						// Else add this call to our list of calls
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

#ifdef _SKYPEAPIDEBUG
				WriteDebug(TEXT("SkypeApiWndProc"), skypeObject);
#endif
			}
			return TRUE;
		}
	}
	// If this is an AttachmentMessage then process it and return TRUE to Skype.
	else if (processAttachmentMessage(message, wParam, lParam))
		return TRUE;

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// A worker-thread which asynchronously from the main thread, sends a request to 
// Skype to retrieve the crucial information about a call object.
DWORD WINAPI SkypeCallGetterThreadProc(__in  LPVOID lpParameter)
{
	COPYDATASTRUCT copyData = {0};
	static int commandId = 1;
	TCHAR str[256];

	SkypeCallObject *callObject = (SkypeCallObject*)lpParameter;
	if (!WaitForSingleObject(hMutex, 100))
	{
		_stprintf_s(str, 256, TEXT("#%d GET CALL %d TYPE"), commandId++, callObject->callId);		
		sendSkypeMessage(str);
		_stprintf_s(str, 256, TEXT("#%d GET CALL %d DURATION"), commandId++, callObject->callId);		
		sendSkypeMessage(str);
		_stprintf_s(str, 256, TEXT("#%d GET CALL %d STATUS"), commandId++, callObject->callId);		
		sendSkypeMessage(str);
		_stprintf_s(str, 256, TEXT("#%d GET CALL %d PARTNER_HANDLE"), commandId++, callObject->callId);		
		sendSkypeMessage(str);
		_stprintf_s(str, 256, TEXT("#%d GET CALL %d PARTNER_DISPNAME"), commandId++, callObject->callId);		
		sendSkypeMessage(str);
		ReleaseMutex(hMutex);

#ifdef _SKYPEAPIDEBUG
		WriteDebug(TEXT("Call Getter thread"), (SkypeObject*)callObject);
#endif
	}
	return 0;
}

// A worker-thread. This thread handles the message queue from Skype. A message must contain all the required 
// data to be passed on to the subscribers, if some of it is missing, the message gets discarded after 3 seconds.
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
					// While callObject exists and skypeCallStatusCallback is set
					for ( ; callObject && skypeCallStatusCallback; )
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

#ifdef _SKYPEAPIDEBUG
					{
						SkypeObject *so;
						listGetValue(callObjectQueue, listSelectFirst(callObjectQueue), &so);
						WriteDebug(TEXT("Queue thread"), so);
					}
#endif
					// Delete the first message in the queue
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

// Monitors if the connection with Skype is still alive.
VOID CALLBACK		PingTimerProc(HWND hWnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
	UNREFERENCED_PARAMETER(hWnd), UNREFERENCED_PARAMETER(message), UNREFERENCED_PARAMETER(idEvent), UNREFERENCED_PARAMETER(dwTime);

	if (skypeApiWindowHandle)
	{
		if (skypePingStatus)
		{
			sendSkypeMessage(TEXT("PING"));
			if (skypeConnectionStatusCallback)
				skypeConnectionStatusCallback(ATTACH_ACTIVE);
		}
		else
			processAttachmentMessage(msgIdApiAttach, 0, ATTACH_CONNECTION_LOST);

		skypePingStatus = 0;
	}
}

#ifdef _SKYPEAPIDEBUG
void WriteDebug(LPTSTR name, SkypeObject *skypeObject)
{
	if (skypeObject)
	{
		switch(skypeObject->object)
		{
			case OBJECT_CALL:
			{
				SkypeCallObject *callObject = (SkypeCallObject *)skypeObject;
				if (!WaitForSingleObject(hMutex, 1000))
				{
					FILE *fp;
					TCHAR buff[1000];
					_tfopen_s(&fp, TEXT("Debug.txt"), TEXT("at"));
					_stprintf_s(buff, 1000, TEXT("%s\tthread: 0x%p\tObject: %d\tCall ID: %d\tCommand ID: %d\tDuration: %d\t\tProperty: %d\t\tStatus: %d\tType: %d")
						TEXT("\tPartner handle: %s\tPartner Display name: %s\n"), name, GetModuleHandle(NULL),
						callObject->object, callObject->callId, callObject->commandId, callObject->duration, callObject->property, callObject->status, callObject->type, callObject->partnerHandle, callObject->partnerDisplayName);
					_fputts(buff, fp);
					fclose(fp);
				}
				ReleaseMutex(hMutex);
			}
			break;
		}
	}
	else
	{
	}
}
#endif