#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "ProcessMonitor.h"
#include "ProcessEntry.h"

int wmain(int argc, wchar_t * argv[], wchar_t * envp[])
{
	auto monitor = new ProcessMonitor();

	std::vector<ProcessEntry> pe = monitor->getProcessesInfo();

	std::sort(pe.begin(), pe.end());

	/*std::sort(pe.begin(), pe.end(), [](const ProcessEntry& left, const ProcessEntry& right)
	{
		return ::lstrcmpiW(left.fileName, right.fileName) < 0;
	});*/

	printf("total IDS: %u\n\n", pe.size());

	for (ProcessEntry p : pe)
	{
		printf("%-50S PID %-10lu PPID %-10lu RUN %-10lu \n", p.fileName, p.processId,
			p.parentProcessId, p.runThreads);
	}

	::system("pause");
	return 0;
}