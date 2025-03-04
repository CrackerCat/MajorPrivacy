#include "pch.h"
#include "HookUtils.h"

#include "./Detours/detours.h"

bool HookFunction(void* pFunction, void* pHook, void** ppOriginal)
{
	DetourTransactionBegin();
	DetourUpdateThread(NtCurrentThread());

	LONG error = DetourAttach((PVOID*)&pFunction, pHook);

	DetourTransactionCommit();

	if (error == NO_ERROR)
	{
		*ppOriginal = pFunction;
		return true;
	}
	return false;
}
