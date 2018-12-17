#include "ProcessEntry.h"

ProcessEntry::ProcessEntry(
	DWORD processId,
	DWORD runThreads,
	DWORD parentProcessId,
	const WCHAR* fileName,
	const WCHAR* userName,
	const WCHAR* domainName,
	int running,
	long long memoryUsage
)
{
	this->processId = processId;
	this->runThreads = runThreads;
	this->parentProcessId = parentProcessId;
	::wcscpy_s(this->fileName, MAX_PATH, fileName);
	::wcscpy_s(this->userName, MAX_PATH, userName);
	::wcscpy_s(this->domainName, MAX_PATH, domainName);
	this->running = running;
	this->memoryUsage = memoryUsage;
}

bool ProcessEntry::operator < (const ProcessEntry& pe) const
{
	return this->processId < pe.processId;
}

