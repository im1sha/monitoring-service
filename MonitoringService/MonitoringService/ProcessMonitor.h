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



class ProcessMonitor
{
public:
	ProcessMonitor();
	~ProcessMonitor();
	bool getProcessesInfo(std::vector<ProcessEntry> * processInfos);
private:
	bool getLogonFromToken(HANDLE hToken, WCHAR* userString, WCHAR* domainString);
	void cleanUp(PTOKEN_USER tokenInformation);
	bool getUserInfoByProcessId(const DWORD procId, WCHAR* userString, 
		WCHAR* domainString, int * running, long long * memoryUsageInMb
	);
	bool getWorkingSetSize(HANDLE hProcess, long long * memoryUsageInMb);
	bool setPrivilege(HANDLE hToken, const WCHAR * lpszPrivilege, BOOL bEnablePrivilege);
};

