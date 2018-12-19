#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <vector>
#include <wchar.h>
#include <string>
#include <iostream>
#include <utility>
#include "Constants.h"

#pragma comment(lib, "pdh.lib")

class CounterAnalizer
{
public:
	CounterAnalizer();
	~CounterAnalizer();

	bool __stdcall collectPerfomanceData(
		std::vector<std::vector<double> >* values, 
		std::vector<WCHAR*>* instances, 
		DWORD collectInterval = 10, 
		size_t totalIntervals = 10
	);

private:
	PDH_COUNTER_PATH_ELEMENTS * __stdcall getPathsToCounter(
		WCHAR * machineName, std::vector<WCHAR*> instances
	);

	bool __stdcall getCounterValues(
		PDH_COUNTER_PATH_ELEMENTS * cpe, 
		const size_t cpeSize, 
		std::vector<std::vector<double> >* resultValues, 
		DWORD collectInterval = 10, 
		size_t totalIntervals = 10
	);
};

