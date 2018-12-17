#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "ProcessMonitor.h"
#include "ProcessEntry.h"

int wmain(int argc, wchar_t * argv[], wchar_t * envp[])
{
	auto monitor = new ProcessMonitor();

	std::vector<ProcessEntry> pe;
	monitor->getProcessesInfo(&pe);

	std::sort(pe.begin(), pe.end());

	/*std::sort(pe.begin(), pe.end(), [](const ProcessEntry& left, const ProcessEntry& right)
	{
		return ::lstrcmpiW(left.fileName, right.fileName) < 0;
	});*/

	::printf("total IDS: %u\n\n", pe.size());

	for (ProcessEntry p : pe)
	{
		::printf("%-40S PID %-10lu PPID %-10lu THR %-7lu  %-17S  MEM %-5lli\n", p.fileName, p.processId,
			p.parentProcessId, p.runThreads, p.userName, p.memoryUsage);
	}

	::system("pause");
	return 0;
}


//
//#include "pch.h"
//#include <iostream>
//#include <windows.h>
//#include <stdio.h>
//#include <tchar.h>
//
//// Use to convert bytes to MB
//#define DIV 1048576
//
//// Use to convert bytes to MB
////#define DIV 1024
//
//// Specify the width of the field in which to print the numbers. 
//// The asterisk in the format specifier "%*I64d" takes an integer 
//// argument and uses it to pad and right justify the number.
//
//#define WIDTH 7
//
//void _tmain()
//{
//	MEMORYSTATUSEX statex;
//
//	statex.dwLength = sizeof(statex);
//
//	GlobalMemoryStatusEx(&statex);
//
//
//	_tprintf(TEXT("There is  %*ld percent of memory in use.\n"), WIDTH, statex.dwMemoryLoad);
//	_tprintf(TEXT("There are %*I64d total Mbytes of physical memory.\n"), WIDTH, statex.ullTotalPhys / DIV);
//	_tprintf(TEXT("There are %*I64d free Mbytes of physical memory.\n"), WIDTH, statex.ullAvailPhys / DIV);
//	_tprintf(TEXT("There are %*I64d total Mbytes of paging file.\n"), WIDTH, statex.ullTotalPageFile / DIV);
//	_tprintf(TEXT("There are %*I64d free Mbytes of paging file.\n"), WIDTH, statex.ullAvailPageFile / DIV);
//	_tprintf(TEXT("There are %*I64d total Mbytes of virtual memory.\n"), WIDTH, statex.ullTotalVirtual / DIV);
//	_tprintf(TEXT("There are %*I64d free Mbytes of virtual memory.\n"), WIDTH, statex.ullAvailVirtual / DIV);
//	_tprintf(TEXT("There are %*I64d free Mbytes of extended memory.\n"), WIDTH, statex.ullAvailExtendedVirtual / DIV);
//
//
//}
//
//
