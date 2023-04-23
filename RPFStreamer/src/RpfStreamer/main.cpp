#include <main.h>
#include "script.h"

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		scriptRegister(hInstance, ScriptMain);
		break;
	case DLL_PROCESS_DETACH:
		if (GetConsoleWindow() && FreeConsole())
			PostMessageA(GetConsoleWindow(), WM_CLOSE, 0, 0);
		scriptUnregister(hInstance);
		break;
	}
	return TRUE;
}