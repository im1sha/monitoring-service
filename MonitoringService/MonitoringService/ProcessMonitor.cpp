#include "ProcessMonitor.h"

ProcessMonitor::ProcessMonitor()
{
	section_ = new CRITICAL_SECTION();
	::InitializeCriticalSectionAndSpinCount(section_, DEFAULT_SPIN_COUNT);
}

ProcessMonitor::~ProcessMonitor()
{
	if (section_ != nullptr)
	{
		::DeleteCriticalSection(section_);
		delete section_;
	}

	while (isRunning())
	{
		::Sleep(1);
	}
}

void __stdcall ProcessMonitor::runAsBackground()
{
	::EnterCriticalSection(section_);
	running_ = true;
	beginThread();
	::LeaveCriticalSection(section_);
}	

void __stdcall ProcessMonitor::shutdown()
{
	::EnterCriticalSection(section_);
	running_ = false;
	::LeaveCriticalSection(section_);
}

void __stdcall ProcessMonitor::setPerfomanceData(std::vector<ProcessInfo> pi)
{
	::EnterCriticalSection(section_);
	perfomanceInfo_ = pi;
	::LeaveCriticalSection(section_);
}

void __stdcall ProcessMonitor::keepTracking(ProcessMonitor * monitor)
{
	while (monitor->isRunning())
	{
		std::vector<ProcessInfo> pi;
		monitor->getProcessesInfo(&pi);
		std::sort(pi.begin(), pi.end());
		monitor->setPerfomanceData(pi);
		::Sleep(DEFAULT_TIMEOUT);
	}
}

void __stdcall ProcessMonitor::beginThread()
{
	thread_ = (HANDLE)0L;

	while (thread_ == (HANDLE)0L)
	{
		thread_ = (HANDLE) ::_beginthreadex(nullptr, 0,
			(_beginthreadex_proc_type)ProcessMonitor::keepTracking,
			(void *)this, 0, nullptr);
		::Sleep(1);
	}
}

std::vector<ProcessInfo> __stdcall ProcessMonitor::getPerfomanceData()
{
	std::vector<ProcessInfo> result;
	::EnterCriticalSection(section_);
	result = perfomanceInfo_;
	::LeaveCriticalSection(section_);
	return result;
}

bool __stdcall ProcessMonitor::getProcessesInfo(
	std::vector<ProcessInfo> * processInfos
)
{
	bool result = true;
	std::vector<DWORD> pids;
	std::vector<DWORD> ppids;
	std::vector<DWORD> threadCounts;
	std::vector<double> workingSet;
	std::vector<double> workingSetPrivate;
	std::vector<double> io;
	std::vector<double> processorUsage;
	std::vector<double> elapsedTime;
	std::vector<std::wstring> processNames;
	
	CountersAnalizer * c = new CountersAnalizer();
	result &= c->getAveragePerfomance(&pids, &ppids, &threadCounts,
		&workingSet, &workingSetPrivate, &io,
		&processorUsage, &elapsedTime,
		&processNames);
	delete c;

	HANDLE currentThreadToken;

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

	for (size_t i = 0; i < pids.size(); i++)
	{
		WCHAR userName[MAX_PATH]{ };
		WCHAR domainName[MAX_PATH]{ };

		this->getLogonDataByPid(pids[i],
			userName, domainName);

		ProcessInfo pi(
			pids[i], threadCounts[i], ppids[i], processNames[i].c_str(),
			userName, domainName, workingSet[i] / MB_TO_BYTE, 
			workingSetPrivate[i] / MB_TO_BYTE, io[i], 
			processorUsage[i], elapsedTime[i]
		);

		processInfos->push_back(pi);
	}

	::RevertToSelf();

	return result;
}


bool __stdcall ProcessMonitor::getLogonDataFromToken(
	HANDLE token,
	WCHAR* userString,
	WCHAR* domainString
)
{
	if (nullptr == token) 
	{
		return false;
	}

	//HMODULE hModule = GetModuleHandleW(NULL);
	//WCHAR path[MAX_PATH];
	//GetModuleFileNameW(hModule, path, MAX_PATH);

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


void __stdcall ProcessMonitor::cleanUp(
	PTOKEN_USER tokenInformation
)
{
	if (tokenInformation != nullptr)
	{
		::HeapFree(::GetProcessHeap(), 0, (LPVOID)tokenInformation);
	}
}

bool __stdcall ProcessMonitor::getLogonDataByPid(
	const DWORD processId,
	WCHAR* userString,
	WCHAR* domainString
)
{
	bool result = false;

	// now not in use:: add SYNCHRONIZE: required to wait for process
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId); //

	if (hProcess == nullptr)
	{
		this->writeLogonDataOnError(userString, domainString);
		return result;
	}
		
	HANDLE processToken = nullptr;

	if (::OpenProcessToken(hProcess, TOKEN_QUERY, &processToken))
	{	
		result = this->getLogonDataFromToken(processToken, userString, domainString);
	}
	else
	{ 
		this->writeLogonDataOnError(userString, domainString);
	}
	
	::CloseHandle(processToken);
	::CloseHandle(hProcess);
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

void __stdcall ProcessMonitor::writeLogonDataOnError(
	WCHAR* userString,
	WCHAR* domainString
)
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

bool __stdcall ProcessMonitor::isRunning()
{
	bool result = false;
	::EnterCriticalSection(section_);
	result = running_;
	::LeaveCriticalSection(section_);
	return result;
}






