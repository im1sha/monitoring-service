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

class SpreadSheet {
public:

	SpreadSheet(HWND hWnd);
	~SpreadSheet();

	static const int MAX_CELLS = 81;
	const int MAX_LINES = 9;
	const int MIN_LINES = 1;

	void update();
	void initialize(/*int rows, int columns, std::vector<std::wstring> strings, LPARAM lParam*/);

	bool isInitialized();
	POINT getMinWindowSize();
	void respondOnTimer();

private:

	HWND hWnd_ = nullptr; // associated HWND
	HDC hDC_ = nullptr; // device context

	WCHAR** tableStrings_ = nullptr;
	std::vector<std::vector<int> > wordsLenghts_;

	LONG minColumnWidth_ = 0;
	LONG firstRowHeight_ = 0;
	LONG defaultColumnWidth_ = 0;

	LONG lineHeight_ = 0;
	int spaceWidth_ = 0;

	int rows_ = 0;
	int columns_ = 0;

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

	/* private methods */
	void deinitialize();
	void draw(int rows, int columns);

	void paintTable(int rows, int columns, int xStep, std::vector<int> ySteps, int totalWidth, std::vector<int> textHeights, wchar_t** strings, HDC wndDC);

	// text paremeters calculating
	std::vector<int> getTextHeights(std::vector<std::vector<int> > lengths, int lineWidth, int charHeight);
	int getMaxLinesInRow(std::vector<std::vector<int>> lengths, int lineWidth);
	void processStrings(std::vector<std::wstring> strings);

	// string processing
	std::wstring deleteExtraDelimiters(const std::wstring s, wchar_t delimiter);
	std::vector<std::wstring> split(const std::wstring stringToSplit, wchar_t delimiter);
	WCHAR** toWcharArray(std::vector<std::wstring> strings);
	void freeWcharStringArray(wchar_t ** arrayToFree, size_t length);

	
};