#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#define INVALID_ID 0

#include <windows.h>
#include <stdio.h>
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
	std::vector<ProcessEntry> getProcessesInfo();
private:
	bool getLogonFromToken(HANDLE hToken, WCHAR* userString, WCHAR* domainString);
	void CleanUp(PTOKEN_USER tokenInformation);
	bool getUserInfoByProcessId(const DWORD procId, WCHAR* userString, 
		WCHAR* domainString, volatile bool * running, volatile SIZE_T * memoryUsage
	);
};

