#include "CountersAnalizer.h"

CountersAnalizer::CountersAnalizer()
{
}


CountersAnalizer::~CountersAnalizer()
{
}


PDH_COUNTER_PATH_ELEMENTS * __stdcall CountersAnalizer::getPathsToCounter(
	const WCHAR * machineName,
	std::vector<WCHAR *> instances, // in format : Process(X)
	const WCHAR * counterName
)
{
	size_t totalInstances = instances.size();

	PDH_COUNTER_PATH_ELEMENTS * cpe = new PDH_COUNTER_PATH_ELEMENTS[totalInstances];

	for (size_t i = 0; i < totalInstances; i++)
	{
		// final formatting:
		//					  "\Process(X#index)\% Processor Time"	
		cpe[i] = { (LPWSTR)machineName, instances[i], nullptr, nullptr, (DWORD)(1), (LPWSTR)counterName };
	}

	return cpe;
}

bool __stdcall CountersAnalizer::getCounterValues(
	PDH_COUNTER_PATH_ELEMENTS * cpe,
	const size_t cpeSize,
	std::vector <std::vector<double> > * resultValues,
	const size_t totalIntervals,
	const DWORD collectInterval	
)
{
	bool result = false;

	PDH_STATUS status;
	HQUERY hQuery;
	HCOUNTER * hCounter = new HCOUNTER[cpeSize];
	PDH_FMT_COUNTERVALUE counterValue;

	if ((status = ::PdhOpenQuery(nullptr, 0, &hQuery)) != ERROR_SUCCESS)
	{
		return result;
	}

	for (size_t i = 0; i < cpeSize; i++)
	{
		WCHAR fullPath[MAX_PATH] = {};
		DWORD pathSize = sizeof(fullPath);

		if ((status = ::PdhMakeCounterPath(&cpe[i], fullPath, &pathSize, 0)) != ERROR_SUCCESS)
		{
			return result;
		}

		if ((status = ::PdhAddEnglishCounter(hQuery, fullPath, 0, &hCounter[i])) != ERROR_SUCCESS) 
		{
			return result;
		}
	}

	for (size_t i = 0; i < cpeSize; i++)
	{
		resultValues->push_back(std::vector<double>());
	}

	for (size_t i = 0; i < totalIntervals + 1; i++)
	{
		::Sleep(collectInterval);

		if ((status = ::PdhCollectQueryData(hQuery)) != ERROR_SUCCESS) { return result; }

		if (i == 0) { continue; }

		for (size_t j = 0; j < cpeSize; j++)
		{
			if ((status = ::PdhGetFormattedCounterValue(hCounter[j], PDH_FMT_DOUBLE,
				nullptr, &counterValue)) != ERROR_SUCCESS)
			{
				wprintf(L"FAILED @ stat=%X , i=%i , j=%i, name=s\n", 
					(int)status, (int)i, (int)j/*, cpe[i].szObjectName*/);
				(*resultValues)[j].push_back(0.0);
				continue;
			}

			// =====DEBUG_INFO=====
			//if (cpe[j].szInstanceName != nullptr)
			//{
			//	wprintf(L"%s\\%s\\%s\t\t : [%3.3f]\n", cpe[j].szObjectName, cpe[j].szCounterName, cpe[j].szInstanceName, counterValue.doubleValue);
			//}
			//else 
			//{
			//	wprintf(L"%s\\%s\t\t : [%3.3f]\n", cpe[j].szObjectName, cpe[j].szCounterName, counterValue.doubleValue);
			//}

			(*resultValues)[j].push_back(counterValue.doubleValue);

		}
	}

	for (size_t i = 0; i < cpeSize; i++)
	{
		::PdhRemoveCounter(hCounter[i]);
	}

	::PdhCloseQuery(hQuery);
	return true;
}

bool __stdcall CountersAnalizer::collectPerfomanceData(
	std::vector<const WCHAR*> counterNames,
	std::vector<std::vector<std::vector<double> > > * values, //  counter_vector< pid_vector < double_vector > >
	std::vector<std::wstring> * processes,
	std::vector<size_t> totalIntervals,
	std::vector<DWORD> collectIntervals
)
{
	if (processes == nullptr)
	{
		throw std::invalid_argument("processes == nullptr");
	}
	if (values == nullptr)
	{
		throw std::invalid_argument("values == nullptr");
	}
	if (counterNames.size() != totalIntervals.size())
	{
		throw std::invalid_argument("counterNames.size() != totalIntervals.size()");
	}
	if (counterNames.size() != collectIntervals.size())
	{
		throw std::invalid_argument("counterNames.size() != collectIntervals.size()");
	}

	std::vector <WCHAR*> * instances = new std::vector<WCHAR*>();

	const WCHAR COUNTER_OBJECT_NAME[] = L"Process";

	PDH_STATUS status = ERROR_SUCCESS;
	volatile LPWSTR counterListBuffer = nullptr;
	DWORD counterListSize = 0;
	volatile LPWSTR instanceListBuffer = nullptr;
	DWORD instanceListSize = 0;



	status = ::PdhEnumObjects(
		nullptr, nullptr, counterListBuffer, &counterListSize, 
		PERF_DETAIL_WIZARD, TRUE
	);

	if (status == PDH_MORE_DATA)
	{
		counterListBuffer = (LPWSTR)calloc(counterListSize, sizeof(WCHAR));
		if (nullptr != counterListBuffer)
		{
			status = ::PdhEnumObjects(
				nullptr, nullptr, counterListBuffer, &counterListSize,
				PERF_DETAIL_WIZARD, TRUE
			);
			if (status != ERROR_SUCCESS) { }
		}
	}


	status = ::PdhEnumObjectItems(nullptr, nullptr, COUNTER_OBJECT_NAME,
		counterListBuffer, &counterListSize, instanceListBuffer,
		&instanceListSize, PERF_DETAIL_WIZARD, 0);

	if (status == PDH_MORE_DATA)
	{
		counterListBuffer = (LPWSTR)calloc(counterListSize, sizeof(WCHAR));
		instanceListBuffer = (LPWSTR)calloc(instanceListSize, sizeof(WCHAR));
	
		if ((nullptr != counterListBuffer) && (nullptr != instanceListBuffer))
		{
			status = ::PdhEnumObjectItems(nullptr, nullptr, COUNTER_OBJECT_NAME,
				counterListBuffer, &counterListSize, instanceListBuffer,
				&instanceListSize, PERF_DETAIL_WIZARD, 0);

			if (status == ERROR_SUCCESS)
			{
				// getting process list
				for (WCHAR * i = instanceListBuffer; *i != 0; i += wcslen(i) + 1)
				{
					processes->push_back(std::wstring(i));
				}
		
				std::sort(processes->begin(), processes->end());

				int currentInstanceNo = 0;
				for (size_t i = 0; i < processes->size(); i++)
				{
					// to get: Process(X#i)
					WCHAR * instance = new WCHAR[MAX_PATH]{};
					if ((i > 0) && ((*processes)[i] == (*processes)[i-1]))
					{
						currentInstanceNo++;
					}
					else
					{
						currentInstanceNo = 0;
					}

					swprintf(instance, MAX_PATH, L"Process(%s#%i)", 
						(*processes)[i].c_str(), currentInstanceNo);
					instances->push_back(instance);
				}

				for (size_t i = 0; i < counterNames.size(); i++)
				{
					values->push_back(std::vector<std::vector<double> >());

					PDH_COUNTER_PATH_ELEMENTS* pcpe = 
						this->getPathsToCounter(nullptr, *instances, counterNames[i]);

					this->getCounterValues(pcpe, instances->size(), 
						&((*values)[i]), totalIntervals[i], 
						collectIntervals[i]);

					if (pcpe != nullptr)
					{
						delete[] pcpe;
					}
				}
			}
			
		}
		else
		{
			status = ERROR_OUTOFMEMORY;
		}
	}

	if (counterListBuffer != nullptr)
	{
		free(counterListBuffer);
	}
	if (instanceListBuffer != nullptr)
	{
		free(instanceListBuffer);
	}
	if (instances != nullptr)
	{
		for (size_t i = 0; i < instances->size(); i++)
		{
			if ((*instances)[i] != nullptr)
			{
				delete[](*instances)[i];
			}
		}
		delete instances;
	}


	return true;
}

double __stdcall CountersAnalizer::getAverage(std::vector<double> values)
{
	size_t size = values.size();
	double sum = 0;
	for (size_t i = 0; i < size; i++)
	{
		sum += values[i];
	}

	return sum / (double)size;
}

std::vector<double> __stdcall CountersAnalizer::normalizeVector(
	std::vector<double> values, 
	size_t idlePosition,
	size_t totalPosition
) 
{
	if (values.size() == 0)
	{
		throw std::invalid_argument("empty vector passed");
	}
	if ((idlePosition > values.size() - 1) || idlePosition < 0)
	{
		throw std::invalid_argument("idlePosition is out of range");
	}
	if ((totalPosition > values.size() - 1) || totalPosition < 0)
	{
		throw std::invalid_argument("totalPosition is out of range");
	}
	
	double idleValue = values[idlePosition];

	double working = 100 - values[idlePosition];
	double passedWorkingSum = 0;

	for (size_t i = 0; i < values.size(); i++)
	{
		if (i != idlePosition && i != totalPosition)
		{
			passedWorkingSum += values[i];
		}
	}

	double normalizeMultiplier = working / passedWorkingSum;
	for (size_t i = 0; i < values.size(); i++)
	{
		if (i != idlePosition && i != totalPosition)
		{
			values[i] *= normalizeMultiplier;
		}
	}
	values[totalPosition] = 100;
	return values;
}

int __stdcall CountersAnalizer::getStringPoistion(
	std::vector<std::wstring> values,
	std::wstring stringToSearch
) 
{
	for (size_t i = 0; i < values.size(); i++)
	{
		if (values[i] == stringToSearch)
		{
			return i;
		}
	}
	return -1;
}

bool __stdcall CountersAnalizer::getAveragePerfomance(
	std::vector<DWORD>* pids,
	std::vector<DWORD>* ppids,
	std::vector<DWORD>* threadCounts,
	std::vector<double>* workingSet,
	std::vector<double>* workingSetPrivate,
	std::vector<double>* io,
	std::vector<double>* processorUsage,
	std::vector<double>* elapsedTime,
	std::vector<std::wstring>* processNames
) 
{
	if (pids == nullptr)
	{
		throw std::invalid_argument("pids == nullptr");
	}
	if (ppids == nullptr)
	{
		throw std::invalid_argument("ppids == nullptr");
	}
	if (threadCounts == nullptr)
	{
		throw std::invalid_argument("threadCounts == nullptr");
	}
	if (workingSet == nullptr)
	{
		throw std::invalid_argument("workingSet == nullptr");
	}
	if (workingSetPrivate == nullptr)
	{
		throw std::invalid_argument("workingSetPrivate == nullptr");
	}
	if (io == nullptr)
	{
		throw std::invalid_argument("io == nullptr");
	}
	if (processorUsage == nullptr)
	{
		throw std::invalid_argument("processorUsage == nullptr");
	}
	if (elapsedTime == nullptr)
	{
		throw std::invalid_argument("elapsedTime == nullptr");
	}
	if (processNames == nullptr)
	{
		throw std::invalid_argument("processNames == nullptr");
	}

	std::vector<std::vector <std::vector<double> > > * values =
		new std::vector<std::vector<std::vector<double> > >();

	std::vector <const WCHAR *> counters{
		COUNTER_PROCESSOR_TIME_PERCENT, // 0
		COUNTER_PROCESS_ID,				// 1
		COUNTER_ELAPSED_TIME,			// 2
		COUNTER_IO_DATA_BYTES_IN_SEC,	// 3
		COUNTER_WORKING_SET,			// 4
		COUNTER_WORKING_SET_PRIVATE,	// 5
		COUNTER_PPID,					// 6
		COUNTER_THREAD_COUNT			// 7
	};

	const DWORD defaultInterval = 10;
	const size_t defaultIntervalTotal = 20;

	std::vector<size_t> totalIntervals{
		defaultIntervalTotal, 1, 1, defaultIntervalTotal, 1, 1, 1, 1
	};

	std::vector<DWORD> intervals{ 
		defaultInterval, defaultInterval, defaultInterval, defaultInterval,
		defaultInterval, defaultInterval, defaultInterval, defaultInterval
	};

	bool result = this->collectPerfomanceData(counters, values, processNames,
		totalIntervals, intervals);

	if (!result)
	{
		return result;
	}

	std::vector<std::vector<double>> averagePerfomance;
	for (size_t i = 0; i < values->size(); i++)
	{
		averagePerfomance.push_back(std::vector<double>());
	}

	for (size_t i = 0; i < values->size(); i++)
	{
		for (size_t j = 0; j < (*values)[i].size(); j++)
		{
			averagePerfomance[i].push_back(this->getAverage((*values)[i][j]));
		}		
	}

	averagePerfomance[0] = this->normalizeVector(averagePerfomance[0],
		this->getStringPoistion(*processNames, PROCESS_IDLE), 
		this->getStringPoistion(*processNames, PROCESS_TOTAL)
	);

	*processorUsage = averagePerfomance[0];  
	*pids = std::vector<DWORD>(averagePerfomance[1].begin(), averagePerfomance[1].end());
	*elapsedTime= averagePerfomance[2];
	*io = averagePerfomance[3];
	*workingSet = averagePerfomance[4];
	*workingSetPrivate = averagePerfomance[5];
	*ppids = std::vector<DWORD>(averagePerfomance[6].begin(), averagePerfomance[6].end());
	*threadCounts = std::vector<DWORD>(averagePerfomance[7].begin(), averagePerfomance[7].end());

	//size_t size = values->size();
	//std::vector<long long> ttt;
	//for (size_t i = 0; i < (*values)[1].size(); i++) //  counter_vector< pid_vector < double_vector > >
	//{
	//	ttt.push_back((long long)(*values)[1][i][0]);
	//}
	//std::sort(ttt.begin(), ttt.end());
	//for (auto i: ttt) //  counter_vector< pid_vector < double_vector > >
	//{
	//	printf("%lli\n", i);
	//}

	delete values;
	return result;
}


