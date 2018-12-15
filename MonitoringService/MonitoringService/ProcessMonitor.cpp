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
		this->getUserByProcessId(procEntry32.th32ProcessID, userName, domainName);

		ProcessEntry pe(procEntry32.th32ProcessID, procEntry32.cntThreads,
			 procEntry32.th32ParentProcessID, procEntry32.szExeFile, userName, domainName);
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

bool ProcessMonitor::getUserByProcessId(
	const DWORD processId,
	WCHAR* userString,
	WCHAR* domainString
)
{
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
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

