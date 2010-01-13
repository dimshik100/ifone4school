// SkypeApiDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SkypeApiDll.h"


// This is an example of an exported variable
SKYPEAPIDLL_API int nSkypeApiDll=0;

// This is an example of an exported function.
SKYPEAPIDLL_API int fnSkypeApiDll(void)
{
	return 42;
}