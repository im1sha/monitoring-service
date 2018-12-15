#include "ProcessMonitor.h"


ProcessMonitor::ProcessMonitor()
{
}


ProcessMonitor::~ProcessMonitor()
{
}


std::vector<DWORD> ProcessMonitor::getProcessesIds()
{
	std::vector<DWORD> ids;

	HANDLE handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 ProcEntry;
	ProcEntry.dwSize = sizeof(PROCESSENTRY32);

	::Process32First(handle, &ProcEntry);

	do
	{
		ids.push_back(ProcEntry.th32ProcessID);
	} while (::Process32Next(handle, &ProcEntry));

	::CloseHandle(handle);

	return ids;
}
