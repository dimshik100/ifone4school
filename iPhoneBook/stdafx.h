// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>

#ifdef UNICODE
#define __strlen wcslen
#define __strcpy_s wcscpy_s
#define __sprintf_s swprintf_s
#else
#define __strlen strlen
#define __strcpy_s strcpy_s
#define __sprintf_s sprintf_s
#endif



// TODO: reference additional headers your program requires here
