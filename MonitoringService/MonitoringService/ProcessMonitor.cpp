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
		//::GetUserNameW(userName, &procEntry32.th32ProcessID);
			   
//		HANDLE processHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, procEntry32.th32ProcessID);
//
//		::OpenProcessToken(
//			processHandle,
//			PROCESS_QUERY_INFORMATION,
//			PHANDLE TokenHandle
//		);
//		//getLogonFromToken()
//
//// 
//		::CloseHandle(processHandle);
//

		ProcessEntry pe(procEntry32.th32ProcessID, procEntry32.cntThreads,
			 procEntry32.th32ParentProcessID, procEntry32.szExeFile, userName);
		infos.push_back(pe);
	} while (::Process32Next(handle, &procEntry32));

	::CloseHandle(handle);

	return infos;
}

BOOL ProcessMonitor::getLogonFromToken(
	HANDLE token,
	_bstr_t& userString,
	_bstr_t& domainString
)
{
	DWORD size = MAX_PATH;
	BOOL isSuccessful = FALSE;
	DWORD length = 0;
	userString = L"";
	domainString = L"";
	PTOKEN_USER tokenUser = nullptr;

	//Verify the parameter passed in is not NULL.
	if (nullptr == token) { goto Cleanup; }
		

	if (!GetTokenInformation(
		token,         // handle to the access token
		TokenUser,    // get information about the token's groups 
		(LPVOID)tokenUser,   // pointer to PTOKEN_USER buffer
		0,              // size of buffer
		&length       // receives required buffer size
	))
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			goto Cleanup;

		tokenUser = (PTOKEN_USER)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY, length);

		if (tokenUser == NULL)
			goto Cleanup;
	}

	if (!GetTokenInformation(
		token,         // handle to the access token
		TokenUser,    // get information about the token's groups 
		(LPVOID)tokenUser,   // pointer to PTOKEN_USER buffer
		length,       // size of buffer
		&length       // receives required buffer size
	))
	{
		goto Cleanup;
	}
	SID_NAME_USE SidType;
	wchar_t lpName[MAX_PATH];
	wchar_t lpDomain[MAX_PATH];

	if (!LookupAccountSid(NULL, tokenUser->User.Sid, lpName, &size, lpDomain, &size, &SidType))
	{
		DWORD dwResult = GetLastError();
		if (dwResult == ERROR_NONE_MAPPED)
			wcscpy_s(lpName, MAX_PATH, L"NONE_MAPPED");
		else
		{
			printf("LookupAccountSid Error %lu\n", GetLastError());
		}
	}
	else
	{
		printf("Current user is  %S\\%S\n",
			lpDomain, lpName);
		userString = lpName;
		domainString = lpDomain;
		isSuccessful = TRUE;
	}

Cleanup:

	if (tokenUser != nullptr) 
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)tokenUser);
	}
	return isSuccessful;
}


void ProcessMonitor::CleanUp() 
{

}

HRESULT ProcessMonitor::GetUserFromProcess(const DWORD procId, _bstr_t& strUser, _bstr_t& strdomain)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, procId);
	if (hProcess == NULL)
		return E_FAIL;
	HANDLE hToken = NULL;

	if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
	{
		CloseHandle(hProcess);
		return E_FAIL;
	}
	BOOL bres = getLogonFromToken(hToken, strUser, strdomain);

	CloseHandle(hToken);
	CloseHandle(hProcess);
	return bres ? S_OK : E_FAIL;
}

