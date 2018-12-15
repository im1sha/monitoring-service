#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "ProcessMonitor.h"

int wmain(int argc, wchar_t * argv[], wchar_t * envp[])
{
	auto pm = new ProcessMonitor();

	std::vector<DWORD> ids = pm->getProcessesIds();
	std::sort(ids.begin(), ids.end());

	printf("total IDS: %u\n\n", ids.size());

	for (DWORD id : ids) 
	{
		printf("%lu\n", id);
	}

	::system("pause");
	return 0;
}