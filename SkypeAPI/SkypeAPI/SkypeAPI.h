#pragma once

#include "resource.h"

typedef enum _SkypeApiInitStatus {	ATTACH_SUCCESS = 0, ATTACH_PENDING = 1, ATTACH_REFUSED = 2,
									ATTACH_NOT_AVAILABLE = 3, ATTACH_AVAILABLE = 0x8001, ATTACH_ACTIVE = 0x9001, 
									ATTACH_CONNECTION_LOST = 0x9002 }
SkypeApiInitStatus;

typedef enum _SkypeApiObjectTypes {	OBJECT_UNKNOWN = 0, 
									OBJECT_USER, OBJECT_PROFILE, OBJECT_CALL, OBJECT_MESSAGE, OBJECT_CHAT,
									OBJECT_CHAT_MEMBER, OBJECT_CHAT_MESSAGE, OBJECT_VOICEMAIL, OBJECT_SMS,
									OBJECT_APPLICATION, OBJECT_GROUP, OBJECT_FILETRANSFER, OBJECT_PONG }
SkypeApiObjectType;

typedef enum _SkypeApiCallProperties {	CALLPROPERTY_UNKNOWN = 0,
										CALLPROPERTY_TIMESTAMP ,CALLPROPERTY_PARTNER_HANDLE ,CALLPROPERTY_PARTNER_DISPNAME,
										CALLPROPERTY_TARGET_IDENTITY, CALLPROPERTY_CONF_ID, CALLPROPERTY_TYPE, CALLPROPERTY_STATUS, 
										CALLPROPERTY_VIDEO_STATUS, CALLPROPERTY_VIDEO_SEND_STATUS, CALLPROPERTY_VIDEO_RECEIVE_STATUS, 
										CALLPROPERTY_FAILUREREASON, CALLPROPERTY_PSTN_NUMBER, CALLPROPERTY_DURATION,
										CALLPROPERTY_PSTN_STATUS, CALLPROPERTY_CONF_PARTICIPANTS_COUNT, CALLPROPERTY_CONF_PARTICIPANT, 
										CALLPROPERTY_VM_DURATION, CALLPROPERTY_VM_ALLOWED_DURATION, CALLPROPERTY_RATE,
										CALLPROPERTY_RATE_CURRENCY, CALLPROPERTY_RATE_PRECISION, CALLPROPERTY_INPUT,
										CALLPROPERTY_OUTPUT, CALLPROPERTY_CAPTURE_MIC, CALLPROPERTY_VAA_INPUT_STATUS, 
										CALLPROPERTY_FORWARDED_BY, CALLPROPERTY_TRANSFER_ACTIVE, CALLPROPERTY_TRANSFER_STATUS,
										CALLPROPERTY_TRANSFERRED_BY, CALLPROPERTY_TRANSFERRED_TO }
SkypeApiCallProperty;

typedef enum _SkypeApiCallTypes {	CALLTYPE_UNKNOWN = 0,
									CALLTYPE_INCOMING_PSTN, CALLTYPE_INCOMING_P2P, 
									CALLTYPE_OUTGOING_PSTN, CALLTYPE_OUTGOING_P2P }
SkypeApiCallType;

typedef enum _SkypeApiCallStatus {	CALLSTATUS_UNKNOWN = 0,
									CALLSTATUS_UNPLACED, CALLSTATUS_ROUTING, CALLSTATUS_IN_PROGRESS,
									CALLSTATUS_ON_HOLD, CALLSTATUS_FINISHED,
									CALLSTATUS_EARLY_MEDIA, CALLSTATUS_FAILED, CALLSTATUS_RINGING, 
									CALLSTATUS_MISSED, CALLSTATUS_REFUSED, CALLSTATUS_BUSY, 
									CALLSTATUS_CANCELLED, CALLSTATUS_TRANSFERRING, CALLSTATUS_TRANSFERRED, 
									CALLSTATUS_WAITING_REDIAL_COMMAND, CALLSTATUS_REDIAL_PENDING, 
									CALLSTATUS_VM_BUFFERING_GREETING, CALLSTATUS_VM_PLAYING_GREETING, 
									CALLSTATUS_VM_UPLOADING, CALLSTATUS_VM_SENT, CALLSTATUS_VM_FAILED,
									CALLSTATUS_VM_CANCELLED, CALLSTATUS_VM_RECORDING  }
SkypeApiCallStatus;

typedef struct _SkypeObject
{
	SkypeApiObjectType	object;
	int					commandId;
}SkypeObject;

typedef struct _SkypeCallObject
{
	SkypeApiObjectType		object;
	int						commandId;
	SkypeApiCallProperty	property;
	SkypeApiCallType		type;
	SkypeApiCallStatus		status;
	LPTSTR					partnerHandle;
	LPTSTR					partnerDisplayName;
	int						callId;
	int						duration;
}SkypeCallObject;

typedef void (CALLBACK* SkypeCallStatusCallback)(SkypeCallObject *skypeCallObject);
typedef void (CALLBACK* SkypeConnectionStatusCallback)(SkypeApiInitStatus skypeApiInitStatus);

BOOL registerSkypeApi(HINSTANCE hInstance);
LRESULT connectSkype(HINSTANCE hInstance);
BOOL processAttachmentMessage(UINT message, WPARAM wParam, LPARAM lParam);
BOOL translateSkypeMessage(WPARAM wParam, LPARAM lParam, SkypeObject **skypeObject);
void disconnectSkype(HINSTANCE hInstance);
UINT getMsgIdApiAttach();
UINT getMsgIdApiDiscover();
HWND getSkypeApiWindowHandle();
void setSkypeCallStatusCallback(SkypeCallStatusCallback skypeCallStatusCallback);
void setSkypeConnectionStatusCallback(SkypeConnectionStatusCallback skypeConnectionStatusCallback);
void call(LPTSTR name);
void hangup(int callId);