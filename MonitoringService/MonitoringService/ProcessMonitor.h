#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#define INVALID_ID 0

#include <windows.h>
#include <stdio.h>
#include <vector>
#include <tlhelp32.h>

class ProcessMonitor
{
public:
	ProcessMonitor();
	~ProcessMonitor();
	std::vector<DWORD> getProcessesIds();
};

