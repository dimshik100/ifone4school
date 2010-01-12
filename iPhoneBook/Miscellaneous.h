#pragma once
#include "DynamicListC.h"

void setImageToDc(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId);
void setImageToDcStretched(HINSTANCE hInstance, RECT *lprc, RECT *lprcOffset, HDC hdc, int imageId);
DynamicListC getContactListLocal();
DynamicListC getContactListFromFile();
DynamicListC getContactListInitiated();