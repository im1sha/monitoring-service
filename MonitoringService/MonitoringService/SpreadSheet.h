#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "GraphicDefines.h"
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <math.h>
#include "ProcessMonitor.h"
#include "ProcessInfo.h"

class SpreadSheet {
public:

	SpreadSheet(HWND hWnd, ProcessMonitor* monitor);
	~SpreadSheet();

	void __stdcall update();
	void __stdcall initialize();
	bool __stdcall isInitialized();
	POINT __stdcall getMinWindowSize();
	void __stdcall respondOnTimer();
	void __stdcall respondOnKeyPress(WPARAM wParam);

	

private:
	
	HWND hWnd_ = nullptr; // associated HWND
	HDC hDC_ = nullptr; // device context

	ProcessMonitor * monitor_ = nullptr;

	std::vector<ProcessInfo> pi_;
	std::vector<std::vector<std::wstring>> content_;
	std::vector<LONG> columnWidths_;

	LONG defaultColumnWidth_;
	LONG minColumnWidth_;

	LONG spaceWidth_;

	LONG lineHeight_;
	
	int currentTopLine_;
	int currentTopRow_;

	bool isInitialized_ = false;

	// font section
	const std::wstring DEFAULT_FONT = L"futuralightc";
	const int DEFAULT_FONT_SIZE = 16;
	HGDIOBJ hPreviousFont_ = nullptr;
	HFONT hFont_ = nullptr;

	// pens
	HPEN hPen_ = nullptr;
	HPEN hOldPen_ = nullptr;

	int oldBackground_;
	COLORREF oldColor_;

	CRITICAL_SECTION* piSection_;

	/* private methods */
	void __stdcall deinitialize();
	void __stdcall draw(std::vector<std::vector<std::wstring>> content);

	void __stdcall paintTable(
		size_t yShift,
		RECT client,
		LONG yStep,
		std::vector<LONG> xSteps,
		LONG totalWidth,
		std::vector<std::vector<wchar_t*>> strings,
		HDC wndDC
	);

	void __stdcall getCellParameters(std::vector<std::vector<std::wstring>> strings);

	std::wstring __stdcall doubleToWstring(double d);

	std::wstring __stdcall dwordToWstring(DWORD d);

	std::vector<std::vector<std::wstring>> __stdcall processInfoVectorToWstrVector(std::vector<ProcessInfo> pi);

	std::vector<wchar_t*> __stdcall toWcharVector(std::vector<std::wstring> strings);

	void __stdcall freeWcharVector(std::vector<wchar_t*> v);

	void __stdcall up();
	void __stdcall down();
	void __stdcall left();
	void __stdcall right();


};

