// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "SkypeAPI.h"

HINSTANCE hInst;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		connectSkype(hModule);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		disconnectSkype(hInst);
		break;
	}
	return TRUE;
}

