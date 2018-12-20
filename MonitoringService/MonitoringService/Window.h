#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include "GraphicDefines.h"
#include "SpreadSheet.h"
#include <string>
#include <vector>
#include <fstream>
#include <codecvt>
#include <sstream>

class Window
{
public:
	Window(HINSTANCE hInstance, int nCmdShow);

	~Window();

	int messageLoop();

private:
	HWND hWnd_ = nullptr;			// related HWND
	HINSTANCE hInstance_ = nullptr; // current instance handle

	// values which identify hWnd_ 
	const WCHAR DEFAULT_CLASS_NAME[20] = L"SpreadSheetClass";
	const WCHAR DEFAULT_WINDOW_NAME[15] = L"SpreadSheet";

	// windows placed on hWnd_
	HWND editRowsHwnd_ = nullptr;
	HWND editColumnsHwnd_ = nullptr;
	HWND buttonOkHwnd_ = nullptr;
	HWND buttonLoadHwnd_ = nullptr;

	const POINT DEFAULT_WINDOW_WIDTH_HEIGHT = { 1700, 900 };

	// child windows default positions 
	const RECT EDIT_ROWS_DEFAULT_POSITION = { 20, 20, 150, 20 };
	const RECT EDIT_COLUMNS_DEFAULT_POSITION = { 20, 60, 150, 20 };
	const RECT BUTTON_OK_DEFAULT_POSITION = { 20, 100, 150, 40 };
	const RECT BUTTON_LOAD_DEFAULT_POSITION = { 20, 160, 150, 40 };

	const LONG DEFAULT_CHILD_STYLE = WS_BORDER | WS_CHILD | WS_VISIBLE;

	const LONG_PTR DEFAULT_OK_NO = 1;	// button OK command LOWORD(wParam) value
	const LONG_PTR DEFAULT_LOAD_NO = 2;	// button LOAD command LOWORD(wParam) value

	const int MAX_TEXT_LENGTH = 20;

	SpreadSheet* spreadSheet_ = nullptr;
	int totalStrings_ = 0;
	std::vector<std::wstring> loadedStrings_;

	HWND initialize(HINSTANCE hInstance, int nCmdShow);
	ATOM registerClass(WNDCLASSEX wndClassEx, HINSTANCE hInstance, WNDPROC wndProc);
	static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool processShowSpreadsheetRequest(LPARAM lParam);
	bool processCreateChildItemsRequest();

	bool loadStringsFromFile();
	bool correctLoadedStrings();
};