#pragma once

#include "resource.h"

typedef enum _SkypeApiInitStatus {	ATTACH_SUCCESS = 0, ATTACH_PENDING = 1, ATTACH_REFUSED = 2,
									ATTACH_NOT_AVAILABLE = 3, ATTACH_AVAILABLE = 0x8001 }
SkypeApiInitStatus;

typedef enum _SkypeApiObjectTypes {	OBJECT_USER, OBJECT_PROFILE, OBJECT_CALL, OBJECT_MESSAGE, OBJECT_CHAT,
								OBJECT_CHAT_MEMBER, OBJECT_CHAT_MESSAGE, OBJECT_VOICEMAIL, OBJECT_SMS,
								OBJECT_APPLICATION, OBJECT_GROUP, OBJECT_FILETRANSFER }
SkypeApiObjectType;

typedef enum _SkypeApiCallStatus {	CALLSTATUS_UNPLACED, CALLSTATUS_ROUTING, CALLSTATUS_IN_PROGRESS,
									CALLSTATUS_ON_HOLD, CALLSTATUS_FINISHED, CALLSTATUS_SEEN,
									CALLSTATUS_DTMF, CALLSTATUS_JOIN_CONFERENCE, CALLSTATUS_DURATION }
SkypeApiCallStatus;

typedef struct _SkypeObject
{
	SkypeApiObjectType object;
}SkypeObject;

typedef struct _SkypeCallObject
{
	SkypeApiObjectType	object;
	SkypeApiCallStatus	status;
	int					callId;
	int					duration;
}SkypeCallObject;

void registerSkypeApi();
LRESULT connectSkype(HWND hWndTarget);
BOOL processAttachmentMessage(UINT message, WPARAM wParam, LPARAM lParam);
BOOL translateSkypeMessage(WPARAM wParam, LPARAM lParam, SkypeObject **skypeObject);
void disconnectSkype();
UINT getMsgIdApiAttach();
UINT getMsgIdApiDiscover();
HWND getSkypeApiWindowHandle();