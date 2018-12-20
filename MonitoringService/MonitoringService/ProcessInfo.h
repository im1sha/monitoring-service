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
		double workingSetInMb,			
		double workingSetPrivateInMb,	
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
	double workingSetInMb;			
	double workingSetPrivateInMb;	
	double io;					// bytes/second	
	double processorUsage;		// %
	double elapsedTime;			// seconds
};

