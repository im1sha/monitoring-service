#include "ProcessInfo.h"

ProcessInfo::ProcessInfo(
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
)
{
	this->processId = processId;
	this->runThreads = runThreads;
	this->parentProcessId = parentProcessId;
	::wcscpy_s(this->fileName, MAX_PATH, fileName);
	::wcscpy_s(this->userName, MAX_PATH, userName);
	::wcscpy_s(this->domainName, MAX_PATH, domainName);
	this->workingSetInMb = workingSetInMb;
	this->workingSetPrivateInMb = workingSetPrivateInMb;
	this->io = io;
	this->processorUsage = processorUsage;
	this->elapsedTime = elapsedTime;
}

bool ProcessInfo::operator < (const ProcessInfo& pe) const
{
	return this->processId < pe.processId;
}

