#pragma once

typedef enum _PbType { PB_OK, PB_OKCANCEL, PB_YESNO } PbType;

INT_PTR promptBox(HWND hwndOwner, LPCTSTR lpText, PbType pbType);