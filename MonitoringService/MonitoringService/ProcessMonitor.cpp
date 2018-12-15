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
		ProcessEntry pe(procEntry32.th32ProcessID, procEntry32.cntThreads,
			procEntry32.th32ParentProcessID, procEntry32.szExeFile);
		infos.push_back(pe);
	} while (::Process32Next(handle, &procEntry32));

	::CloseHandle(handle);

	return infos;
}
