#include "ProcessEntry.h"

ProcessEntry::ProcessEntry(
	DWORD processId,
	DWORD runThreads,
	DWORD parentProcessId,
	const WCHAR* fileName
)
{
	this->processId = processId;
	this->runThreads = runThreads;
	this->parentProcessId = parentProcessId;
	::wcscpy_s(this->fileName, MAX_PATH, fileName);
}

bool ProcessEntry::operator < (const ProcessEntry& pe) const
{
	return this->processId < pe.processId;
}

