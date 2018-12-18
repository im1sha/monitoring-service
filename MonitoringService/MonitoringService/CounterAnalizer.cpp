#include "CounterAnalizer.h"

CounterAnalizer::CounterAnalizer()
{
}


CounterAnalizer::~CounterAnalizer()
{
}

PDH_COUNTER_PATH_ELEMENTS * __stdcall CounterAnalizer::getPathsToCpuCounter(
	WCHAR * machineName,
	std::vector<WCHAR *> instances // in format : Process(X)
)
{
	size_t totalInstances = instances.size();

	PDH_COUNTER_PATH_ELEMENTS * cpe = new PDH_COUNTER_PATH_ELEMENTS[totalInstances];

	for (size_t i = 0; i < totalInstances; i++)
	{
		//"\Process(X)\% Process Time"
		cpe[i] = { machineName, (LPWSTR)instances[i], nullptr, nullptr, i, (LPWSTR)L"% Processor Time" };
	}

	return cpe;
}

bool __stdcall CounterAnalizer::getCounterValues(
	PDH_COUNTER_PATH_ELEMENTS * cpe,
	const size_t cpeSize,
	std::vector <std::vector<double> > * resultValues,
	DWORD collectInterval,
	size_t totalIntervals
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

bool __stdcall CounterAnalizer::collectPerfomanceData(
	std::vector <std::vector<double> > * values,
	std::vector <WCHAR*> * instances,
	DWORD collectInterval,
	size_t totalIntervals
)
{
	const WCHAR COUNTER_OBJECT[] = L"Process";

	PDH_STATUS status = ERROR_SUCCESS;
	LPWSTR counterListBuffer = nullptr;
	DWORD counterListSize = 0;
	LPWSTR instanceListBuffer = nullptr;
	DWORD instanceListSize = 0;

	status = ::PdhEnumObjectItems(nullptr, nullptr, COUNTER_OBJECT,
		counterListBuffer, &counterListSize, instanceListBuffer,
		&instanceListSize, PERF_DETAIL_WIZARD, 0);

	if (status == PDH_MORE_DATA)
	{
		counterListBuffer = (LPWSTR)calloc(counterListSize, sizeof(WCHAR));
		instanceListBuffer = (LPWSTR)calloc(instanceListSize, sizeof(WCHAR));

		if ((nullptr != counterListBuffer) && (nullptr != instanceListBuffer))
		{
			status = ::PdhEnumObjectItems(nullptr, nullptr, COUNTER_OBJECT,
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

				PDH_COUNTER_PATH_ELEMENTS* pcpe = getPathsToCpuCounter(nullptr, *instances);

				getCounterValues(pcpe, instances->size(), values);
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

//int WINAPI collectExample()
//{
//	std::vector <std::vector<double> > * values = new std::vector<std::vector<double> >();
//	std::vector <WCHAR*> * processes = new std::vector <WCHAR*>();
//
//	collectPerfomanceData(values, processes);
//	size_t size = values->size();
//
//	delete values;
//
//	for (size_t i = 0; i < size; i++)
//	{
//		delete[](*processes)[i];
//	}
//
//	delete processes;
//
//	::system("pause");
//
//	return 0;
//}


