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
#include <synchapi.h>
#include <process.h>

#pragma comment(lib, "advapi32.lib")

#include "ProcessInfo.h"
#include "CountersAnalizer.h"

class ProcessMonitor
{
public:
	ProcessMonitor();
	~ProcessMonitor();

	void __stdcall runAsBackground();
	
	std::vector<ProcessInfo> __stdcall getPerfomanceData();

	void __stdcall shutdown();

private:
	void __stdcall setPerfomanceData(std::vector<ProcessInfo> pi);

	static void __stdcall keepTracking(ProcessMonitor * monitor);

	void __stdcall beginThread();

	bool __stdcall getProcessesInfo(
		std::vector<ProcessInfo> * processInfos
	);

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

	bool __stdcall setPrivilege(
		HANDLE token,
		const WCHAR * privilege, 
		BOOL enablePrivilege
	);

	void __stdcall writeLogonDataOnError(
		WCHAR * userString,
		WCHAR * domainString
	);

	const double MB_TO_BYTE = 1048576.0;

	CRITICAL_SECTION * section_ = nullptr;

	std::vector<ProcessInfo> perfomanceInfo_;

	bool running_ = false;

	HANDLE thread_ = nullptr;

	bool __stdcall isRunning();
};

