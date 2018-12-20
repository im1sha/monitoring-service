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

#pragma comment(lib, "advapi32.lib")

#include "ProcessInfo.h"
#include "CountersAnalizer.h"

class ProcessMonitor
{
public:
	ProcessMonitor();
	~ProcessMonitor();

	bool __stdcall getProcessesInfo(
		std::vector<ProcessInfo> * processInfos
	);

private:
	bool __stdcall getLogonDataFromToken(
		HANDLE token, 
		WCHAR* userString,
		WCHAR* domainString
	);

	void __stdcall cleanUp(
		PTOKEN_USER tokenInformation
	);

	bool __stdcall getLogonDataByPid(
		const DWORD procId, 
		WCHAR* userString,
		WCHAR* domainString
	);

	//bool __stdcall getPrivateUsage(HANDLE hProcess, long long * memoryUsageInMb);

	bool __stdcall setPrivilege(
		HANDLE token,
		const WCHAR * privilege, 
		BOOL enablePrivilege
	);

	void __stdcall writeLogonDataOnError(
		WCHAR * userString,
		WCHAR * domainString
	);

	bool _stdcall getCounterStatistics(
		double * workingSet,
		double * workingSetPrivate,
		double * io,
		double * processorUsage,
		double * elapsedTime
	);
};

