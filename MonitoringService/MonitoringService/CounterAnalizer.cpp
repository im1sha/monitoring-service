#include "CounterAnalizer.h"

CounterAnalizer::CounterAnalizer()
{
}


CounterAnalizer::~CounterAnalizer()
{
}


PDH_COUNTER_PATH_ELEMENTS * __stdcall CounterAnalizer::getPathsToCounter(
	const WCHAR * machineName,
	std::vector<WCHAR *> instances, // in format : Process(X)
	const WCHAR * counterName
)
{
	size_t totalInstances = instances.size();

	PDH_COUNTER_PATH_ELEMENTS * cpe = new PDH_COUNTER_PATH_ELEMENTS[totalInstances];

	for (size_t i = 0; i < totalInstances; i++)
	{
		// final formatting: "\Process(X)\% Processor Time"
		//					"\Process(X#index)\% Processor Time"	
		cpe[i] = { (LPWSTR)machineName, instances[i], nullptr, nullptr, i, (LPWSTR)counterName };
	}

	return cpe;
}

bool __stdcall CounterAnalizer::getCounterValues(
	PDH_COUNTER_PATH_ELEMENTS * cpe,
	const size_t cpeSize,
	std::vector <std::vector<double> > * resultValues,
	const DWORD collectInterval,
	const size_t totalIntervals
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

		if ((status = ::PdhAddEnglishCounter(hQuery, fullPath, 0, &hCounter[i])) != ERROR_SUCCESS) {}
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

/////////////

bool __stdcall CounterAnalizer::collectPerfomanceData(
	std::vector<const WCHAR*> counterNames,
	std::vector<std::vector<std::vector<double> > > * values, //  counter_vector< pid_vector < double_vector > >
	std::vector <WCHAR*> * instances,
	const DWORD collectInterval,
	const size_t totalIntervals
)
{
	const WCHAR COUNTER_OBJECT_NAME[] = L"Process";

	PDH_STATUS status = ERROR_SUCCESS;
	LPWSTR counterListBuffer = nullptr;
	DWORD counterListSize = 0;
	LPWSTR instanceListBuffer = nullptr;
	DWORD instanceListSize = 0;

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
				for (WCHAR * i = instanceListBuffer; *i != 0; i += wcslen(i) + 1)
				{
					// to get:    Process(X)
					WCHAR *instance = new WCHAR[MAX_PATH]{};
					WCHAR begin[] = L"Process(";
					size_t len = wcslen(begin);
					wcscpy_s(instance, MAX_PATH, begin);
					wcscpy_s(instance + len, MAX_PATH - len, i);
					wcscpy_s(instance + len + wcslen(i), MAX_PATH - len - wcslen(i), L")");

					instances->push_back(instance);
				}

				for (size_t i = 0; i < counterNames.size(); i++)
				{
					values->push_back(std::vector<std::vector<double> >());

					PDH_COUNTER_PATH_ELEMENTS* pcpe = 
						this->getPathsToCounter(nullptr, *instances, counterNames[i]);

					this->getCounterValues(pcpe, instances->size(), &((*values)[i]));
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

	return true;
}

int __stdcall CounterAnalizer::collectExample()
{
	std::vector<std::vector <std::vector<double> > > * values = new std::vector<std::vector<std::vector<double> > >();
	std::vector <WCHAR*> * processNames = new std::vector <WCHAR*>();
	std::vector <const WCHAR *> counters;
	counters.push_back(COUNTER_PROCESSOR_TIME_PERCENT);
	counters.push_back(COUNTER_PROCESS_ID);
	counters.push_back(COUNTER_ELAPSED_TIME);
	counters.push_back(COUNTER_IO_DATA_BYTES_IN_SEC);
	counters.push_back(COUNTER_WORKING_SET);
	counters.push_back(COUNTER_WORKING_SET_PRIVATE);

	this->collectPerfomanceData(counters, values, processNames);

	size_t size = values->size();
	std::vector<long long> ttt;
	for (size_t i = 0; i < (*values)[1].size(); i++) //  counter_vector< pid_vector < double_vector > >
	{
		ttt.push_back((*values)[1][i][0]);
	}
	std::sort(ttt.begin(), ttt.end());

	for (auto i: ttt) //  counter_vector< pid_vector < double_vector > >
	{
		printf("%lli\n", i);
	}

	delete values;

	for (size_t i = 0; i < size; i++)
	{
		delete[](*processNames)[i];
	}

	delete processNames;

	::system("pause");

	return 0;
}


