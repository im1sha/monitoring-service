#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include "ProcessMonitor.h"
#include "ProcessInfo.h"

int wmain(int argc, wchar_t * argv[], wchar_t * envp[])
{
	auto monitor = new ProcessMonitor();

	std::vector<ProcessInfo> pi;

	//DWORD now1 = GetTickCount();

	

	/*
	std::sort(pe.begin(), pe.end(), [](const ProcessEntry& left, const ProcessEntry& right)
	{
		return ::lstrcmpiW(left.fileName, right.fileName) < 0;
	});
	*/

	/*ProcessInfo::ProcessInfo(
		DWORD processId,
		DWORD runThreads,
		DWORD parentProcessId,
		double workingSet,
		double workingSetPrivate,
		double io,
		double processorUsage,
		double elapsedTime
	)*/
	//for (ProcessInfo p : pi)
		//{
		//	::printf("%-40S PID %-10lu PPID %-10lu THR %-7lu  %-17S  %-17S  WS %-10.0f WSP %-10.0f IO %-5.0f CPU %-5.2f TIM %-10.0f\n", 
		//		p.fileName, p.processId, p.parentProcessId, p.runThreads, 
		//		p.userName, p.domainName, p.workingSetInMb, p.workingSetPrivateInMb, p.io, p.processorUsage, p.elapsedTime);
		//}

	for (size_t i = 0; i < 200; i++)
	{	
		pi = std::vector<ProcessInfo>();
		bool res =monitor->getProcessesInfo(&pi);
		std::sort(pi.begin(), pi.end());
		::printf("\n\n========================================================================\n\n");	
		/*wprintf(L"%s %f\n", (pi)[0].fileName, (pi)[0].processorUsage);
		wprintf(L"%s %f\n\n", (pi)[1].fileName, (pi)[1].processorUsage);*/
		printf("TOTAL PROCESSES %i %i \n\n", pi.size(), res?1:0);

		for (ProcessInfo p : pi)
		{
			::printf("%-40S PID %-10lu PPID %-10lu THR %-7lu  %-17S  %-17S  WS %-10.0f WSP %-10.0f IO %-5.0f CPU %-5.2f TIM %-10.0f\n", 
				p.fileName, p.processId, p.parentProcessId, p.runThreads, 
				p.userName, p.domainName, p.workingSetInMb, p.workingSetPrivateInMb, p.io, p.processorUsage, p.elapsedTime);
		}

	}

	//CountersAnalizer * c = new CountersAnalizer();
	//std::vector<DWORD>* pids = new std::vector<DWORD>();
	//std::vector<DWORD>* ppids = new std::vector<DWORD>();
	//std::vector<DWORD>* threadCounts = new std::vector<DWORD>();
	//std::vector<double>* workingSet = new std::vector<double>();
	//std::vector<double>* workingSetPrivate = new std::vector<double>();
	//std::vector<double>* io = new std::vector<double>();
	//std::vector<double>* processorUsage = new std::vector<double>();
	//std::vector<double>* elapsedTime = new std::vector<double>();
	//std::vector<std::wstring>* processNames = new std::vector<std::wstring>();
	//c->getAveragePerfomance(pids, ppids, threadCounts,
	//	workingSet, workingSetPrivate, io,
	//	processorUsage, elapsedTime,
	//	processNames);
	//delete pids;
	//delete ppids;
	//delete threadCounts;
	//delete workingSet;
	//delete workingSetPrivate;
	//delete io;
	//delete processorUsage;
	//delete elapsedTime;
	//delete processNames;
	//delete c;


	/*DWORD now2 = GetTickCount();

	printf("TOTAL: %lu\n", now2 - now1);*/
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
