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
#include "ProcessEntry.h"

class ProcessMonitor
{
public:
	ProcessMonitor();
	~ProcessMonitor();
	std::vector<ProcessEntry> getProcessesInfo();
private:
	BOOL getLogonFromToken(HANDLE hToken, _bstr_t & strUser, _bstr_t & strdomain);
	void CleanUp();
	HRESULT GetUserFromProcess(const DWORD procId, _bstr_t & strUser, _bstr_t & strdomain);
};

