#pragma once
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include <windows.h>
#include <string.h>

struct ProcessInfo
{
	ProcessInfo(
		DWORD processId,
		DWORD runThreads,
		DWORD parentProcessId,
		const WCHAR* fileName,
		const WCHAR* userName,
		const WCHAR* domainName,
		double workingSet,			
		double workingSetPrivate,	
		double io,				
		double processorUsage,	
		double elapsedTime		
	);

	bool operator < (const ProcessInfo& pe) const;

	DWORD processId;
	DWORD parentProcessId;
	DWORD runThreads;
	WCHAR fileName[MAX_PATH];
	WCHAR userName[MAX_PATH];
	WCHAR domainName[MAX_PATH];
	double workingSet;			// in bytes
	double workingSetPrivate;	// in bytes
	double io;					// bytes/second	
	double processorUsage;		// %
	double elapsedTime;			// seconds
};

//const wchar_t COUNTER_PROCESSOR_TIME_PERCENT[17] = L"% Processor Time";
//const wchar_t COUNTER_PROCESS_ID[11] = L"ID Process";
//const wchar_t COUNTER_ELAPSED_TIME[13] = L"Elapsed Time";
//const wchar_t COUNTER_IO_DATA_BYTES_IN_SEC[18] = L"IO Data Bytes/sec";
//const wchar_t COUNTER_WORKING_SET[12] = L"Working Set";
//const wchar_t COUNTER_WORKING_SET_PRIVATE[22] = L"Working Set - Private";
