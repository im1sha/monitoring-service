#include "ProcessMonitor.h"

ProcessMonitor::ProcessMonitor()
{
}

ProcessMonitor::~ProcessMonitor()
{
}

bool __stdcall ProcessMonitor::getProcessesInfo(std::vector<ProcessEntry> * processInfos)
{
	HANDLE currentThreadToken;
	HANDLE snapshotHandle = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	
	if (snapshotHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	PROCESSENTRY32 procEntry32;
	procEntry32.dwSize = sizeof(PROCESSENTRY32);

	if (!::Process32First(snapshotHandle, &procEntry32))
	{
		::CloseHandle(snapshotHandle);
		return false;
	}

	if (!::OpenThreadToken(::GetCurrentThread(), 
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		FALSE, &currentThreadToken))
	{
		if (::GetLastError() == ERROR_NO_TOKEN)
		{
			if (!::ImpersonateSelf(SecurityImpersonation)) 
			{
				return false;
			}			
			if (!::OpenThreadToken(GetCurrentThread(), 
				TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
				FALSE, &currentThreadToken)) 
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	this->setPrivilege(currentThreadToken, SE_DEBUG_NAME, TRUE);

	do
	{		
		WCHAR userName[MAX_PATH] {};
		WCHAR domainName[MAX_PATH] {};
		int running = -2;
		long long memoryUsage = -2;

		this->getUserInfoByProcessId(procEntry32.th32ProcessID, 
			userName, domainName, &running, &memoryUsage);

		ProcessEntry pe(procEntry32.th32ProcessID, procEntry32.cntThreads,
			procEntry32.th32ParentProcessID, procEntry32.szExeFile, 
			userName, domainName, running, memoryUsage);

		processInfos->push_back(pe);

	} while (::Process32Next(snapshotHandle, &procEntry32));

	::CloseHandle(snapshotHandle);

	::RevertToSelf();

	return true;
}

bool __stdcall ProcessMonitor::getLogonFromToken(
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
		this->cleanUp(tokenInformation);
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


void __stdcall ProcessMonitor::cleanUp(PTOKEN_USER tokenInformation)
{
	if (tokenInformation != nullptr)
	{
		::HeapFree(::GetProcessHeap(), 0, (LPVOID)tokenInformation);
	}
}

bool __stdcall ProcessMonitor::getUserInfoByProcessId(
	const DWORD processId,
	WCHAR* userString,
	WCHAR* domainString,
	int * running,
	long long * memoryUsageInMb
)
{
	*memoryUsageInMb = -1;
	*running = -1;

	// mb:: add SYNCHRONIZE: required to wait for process
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId); //

	if (hProcess == nullptr)
	{
		this->writeUsernameAndDomainOnError(userString, domainString);
		return false;
	}
		
	HANDLE processToken = nullptr;

	bool result1 = this->getPrivateUsage(hProcess, memoryUsageInMb);

	if (::OpenProcessToken(hProcess, TOKEN_QUERY, &processToken))
	{	
		result1 &= this->getLogonFromToken(processToken, userString, domainString);
	}
	else
	{ 
		this->writeUsernameAndDomainOnError(userString, domainString);
	}
	
	::CloseHandle(processToken);
	::CloseHandle(hProcess);
	return result1;
}

bool __stdcall ProcessMonitor::getPrivateUsage(HANDLE hProcess, long long * memoryUsageInMb)
{
	bool result = false;

	const long long bytesInMb = (long long)1024 * 1024;

	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);

	if (::GetProcessMemoryInfo(hProcess, (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc)))
	{
		*memoryUsageInMb = (long long)pmc.PrivateUsage / bytesInMb;
		result = true;
	}

	return result;
}

bool __stdcall ProcessMonitor::setPrivilege(
	HANDLE token,				// access token handle
	const WCHAR * privilege,	// name of privilege to enable/disable
	BOOL enablePrivilege		// to enable or disable privilege
)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!::LookupPrivilegeValue(nullptr, privilege, &luid))       
	{
		return false;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;

	if (enablePrivilege) 
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else 
	{
		tp.Privileges[0].Attributes = 0;
	}

	if (!::AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)nullptr, (PDWORD)nullptr))
	{

		return false;
	}

	if (::GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{

		return false;
	}
	return true;
}

void __stdcall ProcessMonitor::writeUsernameAndDomainOnError(WCHAR* userString, WCHAR* domainString)
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



