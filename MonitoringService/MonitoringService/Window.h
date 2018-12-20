#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "GraphicDefines.h"
#include "SpreadSheet.h"
#include "ProcessMonitor.h"

class Window
{
public:
	Window(HINSTANCE hInstance, int nCmdShow);

	~Window();

	int __stdcall messageLoop();

private:
	HWND hWnd_ = nullptr;			// related HWND
	HINSTANCE hInstance_ = nullptr; // current instance handle

	// values which identify hWnd_ 
	const WCHAR DEFAULT_CLASS_NAME[17] = L"SpreadSheetClass";
	const WCHAR DEFAULT_WINDOW_NAME[12] = L"SpreadSheet";
	const POINT DEFAULT_WINDOW_WIDTH_HEIGHT = { 1600, 900 };

	SpreadSheet* spreadSheet_ = nullptr;
	ProcessMonitor* monitor_ = nullptr;

	bool __stdcall processCreationRequest();
	HWND __stdcall initialize(HINSTANCE hInstance, int nCmdShow);
	ATOM __stdcall registerClass(WNDCLASSEX wndClassEx, HINSTANCE hInstance, WNDPROC wndProc);
	static LRESULT __stdcall windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
};