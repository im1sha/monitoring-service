#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#define INVALID_ID 0

#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "advapi32.lib")
#include <vector>
#include <tlhelp32.h>
#include <comdef.h>
#include <psapi.h>
#include "ProcessEntry.h"
#include "CounterAnalizer.h"

class ProcessMonitor
{
public:
	ProcessMonitor();
	~ProcessMonitor();
	bool __stdcall getProcessesInfo(std::vector<ProcessEntry> * processInfos);
private:
	bool __stdcall getLogonFromToken(HANDLE hToken, WCHAR* userString, WCHAR* domainString);
	void __stdcall cleanUp(PTOKEN_USER tokenInformation);
	bool __stdcall getUserInfoByProcessId(const DWORD procId, WCHAR* userString,
		WCHAR* domainString, int * running, long long * memoryUsageInMb
	);
	bool __stdcall getPrivateUsage(HANDLE hProcess, long long * memoryUsageInMb);
	bool __stdcall setPrivilege(HANDLE hToken, const WCHAR * lpszPrivilege, BOOL bEnablePrivilege);
	void __stdcall writeUsernameAndDomainOnError(WCHAR * userString, WCHAR * domainString);
};

