#include "ProcessMonitor.h"

ProcessMonitor::ProcessMonitor()
{
}

ProcessMonitor::~ProcessMonitor()
{
}

std::vector<ProcessEntry> ProcessMonitor::getProcessesInfo()
{
	std::vector<ProcessEntry> infos;

	HANDLE handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // <

	PROCESSENTRY32 procEntry32;
	procEntry32.dwSize = sizeof(PROCESSENTRY32);

	::Process32First(handle, &procEntry32);

	do
	{		
		WCHAR userName[MAX_PATH] {};
		WCHAR domainName[MAX_PATH] {};
		volatile bool running;
		volatile SIZE_T memoryUsage;

		this->getUserInfoByProcessId(procEntry32.th32ProcessID, 
			userName, domainName, &running, &memoryUsage);

		ProcessEntry pe(procEntry32.th32ProcessID, procEntry32.cntThreads,
			procEntry32.th32ParentProcessID, procEntry32.szExeFile, 
			userName, domainName, running, memoryUsage);

		infos.push_back(pe);

	} while (::Process32Next(handle, &procEntry32));

	::CloseHandle(handle);

	return infos;
}

bool ProcessMonitor::getLogonFromToken(
	HANDLE token,
	WCHAR* userString,
	WCHAR* domainString
)
{
	if (nullptr == token) 
	{
		return false;
	}

	DWORD maxSize = MAX_PATH;
	DWORD length = 0;
	PTOKEN_USER tokenInformation = nullptr;

	if (!::GetTokenInformation(token, TokenUser, (LPVOID)tokenInformation, 0, &length))
	{
		if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
		{ 
			return false;
		}
		tokenInformation = (PTOKEN_USER)::HeapAlloc(::GetProcessHeap(), 
			HEAP_ZERO_MEMORY, length);
		if (tokenInformation == nullptr) 
		{ 
			return false;
		}
	}

	if (!::GetTokenInformation(token, TokenUser, (LPVOID)tokenInformation, length, &length)) 
	{ 
		this->CleanUp(tokenInformation);
		return false;
	}

	SID_NAME_USE sidType;
	WCHAR name[MAX_PATH];
	WCHAR domain[MAX_PATH];

	if (!::LookupAccountSid(nullptr, tokenInformation->User.Sid, 
		name, &maxSize, domain, &maxSize, &sidType))
	{
		if (::GetLastError() == ERROR_NONE_MAPPED)
		{
			::wcscpy_s(name, MAX_PATH, L"NONE_MAPPED");
		}
	}
	else
	{
		::wcscpy_s(userString, MAX_PATH, name);
		::wcscpy_s(domainString, MAX_PATH, domain);
		return true;
	}
	return false;
}


void ProcessMonitor::CleanUp(PTOKEN_USER tokenInformation)
{
	if (tokenInformation != nullptr)
	{
		::HeapFree(::GetProcessHeap(), 0, (LPVOID)tokenInformation);
	}
}

bool ProcessMonitor::getUserInfoByProcessId(
	const DWORD processId,
	WCHAR* userString,
	WCHAR* domainString,
	volatile bool * running, 
	volatile SIZE_T * memoryUsageInMb
)
{
	*memoryUsageInMb = 0;
	*running = true;

	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ, FALSE, processId);
	if (hProcess == nullptr) 
	{
		::wcscpy_s(domainString, MAX_PATH, L"-");

		if (::GetLastError() == ERROR_INVALID_PARAMETER)
		{
			::wcscpy_s(userString, MAX_PATH, L"System");			
		}
		else if (::GetLastError() == ERROR_ACCESS_DENIED)
		{
			::wcscpy_s(userString, MAX_PATH, L"Idle/CSRSS");
		}
		return false; 
	}

	DWORD waitResult = ::WaitForSingleObject(hProcess, (DWORD)0);

	if (waitResult != WAIT_OBJECT_0)
	{
		*running = false;
	}
			
	const SIZE_T bytesInMb = (SIZE_T) 1024*1024;
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
	if (::GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		*memoryUsageInMb = pmc.WorkingSetSize / bytesInMb;
	}

	
	HANDLE token = nullptr;

	if (!::OpenProcessToken(hProcess, TOKEN_QUERY, &token))
	{
		::CloseHandle(hProcess);
		return false;
	}

	bool result = this->getLogonFromToken(token, userString, domainString);

	::CloseHandle(token);
	::CloseHandle(hProcess);

	return result;
}



//float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
//{
//	static unsigned long long _previousTotalTicks = 0;
//	static unsigned long long _previousIdleTicks = 0;
//
//	unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
//	unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;
//
//	float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);
//
//	_previousTotalTicks = totalTicks;
//	_previousIdleTicks = idleTicks;
//	return ret;
//}
//
//unsigned long long FileTimeToInt64(const FILETIME & ft) 
//{ 
//	return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
//}
//
//// Returns 1.0f for "CPU fully pinned", 0.0f for "CPU idle", or somewhere in between
//// You'll need to call this at regular intervals, since it measures the load between
//// the previous call and the current one.  Returns -1.0 on error.
//float GetCPULoad()
//{
//	FILETIME idleTime, kernelTime, userTime;
//	return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
//}
//
