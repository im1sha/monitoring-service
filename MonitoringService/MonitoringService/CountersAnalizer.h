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
#include <algorithm>

#include "Constants.h"

#pragma comment(lib, "pdh.lib")

class CountersAnalizer
{
public:
	CountersAnalizer();
	~CountersAnalizer();
	bool __stdcall getAveragePerfomance(
		std::vector<DWORD>* pids,
		std::vector<DWORD>* ppids,
		std::vector<DWORD>* threadCounts,
		std::vector<double>* workingSet,
		std::vector<double>* workingSetPrivate,
		std::vector<double>* io,
		std::vector<double>* processorUsage,
		std::vector<double>* elapsedTime,
		std::vector<std::wstring>* processName
	);

private:

	bool __stdcall collectPerfomanceData(
		std::vector<const WCHAR*> counterNames,
		std::vector<std::vector <std::vector<double> > > * values,
		std::vector<std::wstring>* processesNames,
		std::vector<size_t> totalIntervals,
		std::vector<DWORD> collectIntervals		
	);

	double __stdcall getAverage(std::vector<double> values);

	std::vector<double> __stdcall normalizeVector(
		std::vector<double> values, 
		size_t idlePosition, 
		size_t totalPosition
	);

	int __stdcall getStringPoistion(
		std::vector<std::wstring> values, 
		std::wstring stringToSearch
	);
	
	PDH_COUNTER_PATH_ELEMENTS * __stdcall getPathsToCounter(
		const WCHAR * machineName,
		std::vector<WCHAR*> instances, // in format : Process(X)
		const WCHAR* counterName
	);

	bool __stdcall getCounterValues(
		PDH_COUNTER_PATH_ELEMENTS * cpe, 
		const size_t cpeSize, 
		std::vector<std::vector<double> >* resultValues, 
		const size_t totalIntervals = 10,
		const DWORD collectInterval = 10		
	);
};

