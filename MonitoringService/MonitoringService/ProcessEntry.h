#pragma once
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include <windows.h>
#include <string.h>

struct ProcessEntry
{
	ProcessEntry(
		DWORD processId,
		DWORD runThreads,
		DWORD parentProcessId,
		const WCHAR* fileName,
		const WCHAR* userName
	);

	bool operator < (const ProcessEntry& pe) const;

	DWORD processId;
	DWORD runThreads;
	DWORD parentProcessId;
	WCHAR fileName[MAX_PATH];
	WCHAR userName[MAX_PATH];
};

