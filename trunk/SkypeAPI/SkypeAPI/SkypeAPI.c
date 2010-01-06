
#include "stdafx.h"
#include "SkypeAPI.h"

UINT msgIdApiAttach = 0; //RegisterWindowMessage(TEXT("SkypeControlAPIAttach"));
UINT msgIdApiDiscover = 0; //RegisterWindowMessage(TEXT("SkypeControlAPIDiscover"));
HWND skypeApiWindowHandle = NULL;

LPTSTR getStringFromMessage(PCOPYDATASTRUCT copyData);

void registerSkypeApi()
{
	msgIdApiAttach = RegisterWindowMessage(TEXT("SkypeControlAPIAttach"));
	msgIdApiDiscover = RegisterWindowMessage(TEXT("SkypeControlAPIDiscover"));
}

LRESULT connectSkype(HWND hWndTarget)
{
	if (!msgIdApiAttach || !msgIdApiDiscover)
		registerSkypeApi();
	return SendMessage(HWND_BROADCAST, msgIdApiDiscover, (WPARAM)hWndTarget, 0);
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