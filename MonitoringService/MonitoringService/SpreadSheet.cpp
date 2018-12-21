#pragma once

#include "SpreadSheet.h"

SpreadSheet::SpreadSheet(HWND hWnd, ProcessMonitor* monitor)
{
	this->hWnd_ = hWnd;
	this->monitor_ = monitor;
	if (this->monitor_ != nullptr)
	{
		this->monitor_->runAsBackground();
	}
}

SpreadSheet::~SpreadSheet()
{
	if (this->monitor_ != nullptr)
	{
		this->monitor_->shutdown();
	}
	this->deinitialize();
}

void __stdcall SpreadSheet::initialize()
{
	HDC winDC = ::GetDC(hWnd_);

	defaultColumnWidth_ = ::GetSystemMetrics(SM_CXSCREEN) / 10;

	hPen_ = ::CreatePen(PS_SOLID, 1, RGB(200, 100, 200));
	hOldPen_ = (HPEN) ::SelectObject(winDC, hPen_);

	oldBackground_ = ::SetBkMode(winDC, TRANSPARENT);

	hFont_ = ::CreateFont(DEFAULT_FONT_SIZE, NULL, NULL, NULL,
		FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_SWISS, DEFAULT_FONT.c_str());
	hPreviousFont_ = ::SelectObject(winDC, hFont_);
	oldColor_ = ::SetTextColor(winDC, RGB(0, 0, 0));

	// retrieve space width
	SIZE spaceSize = {};
	::GetTextExtentPoint32(winDC, L" ", 1, &spaceSize);
	spaceWidth_ = spaceSize.cx;

	::ReleaseDC(hWnd_, winDC);

	isInitialized_ = true;
}

void __stdcall SpreadSheet::deinitialize()
{
	isInitialized_ = false;

	// restoring previous graphical settings
	HDC wndDC = ::GetDC(hWnd_);

	if (hDC_ != nullptr && hPreviousFont_ != nullptr)
	{
		::SelectObject(hDC_, hPreviousFont_);
		if (hFont_ != nullptr)
		{
			::DeleteObject(hFont_);
		}
	}

	::SetBkMode(wndDC, oldBackground_);
	::SetTextColor(wndDC, oldColor_);

	if (hOldPen_ != nullptr && wndDC != nullptr)
	{
		::SelectObject(wndDC, hOldPen_);
		if (hPen_ != nullptr)
		{
			::DeleteObject(hPen_);
		}
	}

	::ReleaseDC(hWnd_, wndDC);

	// free memory
	delete this->monitor_;
}

bool __stdcall SpreadSheet::isInitialized()
{
	return isInitialized_;
}

POINT __stdcall SpreadSheet::getMinWindowSize()
{
	RECT clientSize;
	RECT windowSize;

	POINT deltaSize; // == windowSize - clientSize

	GetClientRect(hWnd_, &clientSize);
	GetWindowRect(hWnd_, &windowSize);

	deltaSize.x = (windowSize.right - windowSize.left) - clientSize.right;
	deltaSize.y = (windowSize.bottom - windowSize.top) - clientSize.bottom;

	return { minColumnWidth_ + deltaSize.x, lineHeight_ + deltaSize.y };
}

void __stdcall SpreadSheet::respondOnTimer()
{
	this->pi_ = this->monitor_->getPerfomanceData();
	update();
}

void __stdcall SpreadSheet::respondOnKeyPress(WPARAM wParam)
{
	switch (wParam)
	{
	case VK_LEFT:
	{
		left();
		break;
	}
	case VK_RIGHT:
	{
		right();
		break;
	}
	case VK_DOWN:
	{	
		down();
		break;	
	}
	case VK_UP:
	{
		up();
		break;
	}
	}
	update();
}

void __stdcall SpreadSheet::up()
{
	
}

void __stdcall SpreadSheet::down()
{

}

void __stdcall SpreadSheet::left()
{

}

void __stdcall SpreadSheet::right()
{

}

// updates table represetation
void __stdcall SpreadSheet::update()
{
	if (isInitialized_)
	{
		this->draw();
	}
}

// draws table and fill it with content
void __stdcall SpreadSheet::draw()
{
	// client window's data gathering
	HDC wndDC = ::GetDC(hWnd_);

	RECT clientRect;
	::GetClientRect(hWnd_, &clientRect);

	tagPAINTSTRUCT ps {
		ps.hdc = wndDC,
		ps.fErase = true,
		ps.rcPaint = clientRect
	};

	// draw parameters initializing
	::BeginPaint(hWnd_, &ps);
	::SelectObject(wndDC, hPen_);
	::SelectObject(wndDC, hFont_);
	::SetBkMode(wndDC, TRANSPARENT);
	::SetTextColor(wndDC, RGB(46, 23, 150));

	// width calculating
	int averageColumnWidth = (clientRect.right - clientRect.left) / columns;
	int xStep = (averageColumnWidth < minColumnWidth_) ? minColumnWidth_ : averageColumnWidth; /* column width */

	// rows height calculating
	std::vector<int> textHeights = this->getTextHeights(wordsLenghts_, xStep, lineHeight_);
	firstRowHeight_ = textHeights[0];
	int fullTextHeight = 0;
	for (size_t i = 0; i < rows; i++)
	{
		fullTextHeight += textHeights[i];
	}

	std::vector<int> ySteps(rows);

	if (clientRect.bottom - clientRect.top > fullTextHeight)
	{
		for (size_t i = 0; i < rows; i++)
		{
			ySteps[i] = textHeights[i] + (clientRect.bottom - clientRect.top - fullTextHeight) / rows;
		}
		ySteps[ySteps.size() - 1] += (clientRect.bottom - clientRect.top - fullTextHeight) % rows;
	}
	else
	{
		ySteps = textHeights;
	}

	// table painting
	this->paintTable(rows, columns, xStep, ySteps, clientRect.right - clientRect.left, textHeights, tableStrings_, wndDC);

	// draw ending
	::EndPaint(hWnd_, &ps);
	::ReleaseDC(hWnd_, wndDC);
}

// paints table 
void __stdcall SpreadSheet::paintTable(
	LONG xStep,
	std::vector<LONG> ySteps,
	int totalWidth, 
	std::vector<std::vector<std::wstring>> strings, 
	HDC wndDC
)
{
	int currentBottom = 0;

	for (size_t j = 0; j < rows; j++)
	{
		int yStep = ySteps[j];
		currentBottom += yStep;
		for (size_t i = 0; i < columns; i++)
		{
			wchar_t* textToPrint = strings[j * columns + i];

			RECT cell = {
				(LONG)(xStep * i),
				(LONG)(currentBottom - ySteps[j]),
				(LONG)(xStep * (i + 1)),
				(LONG)(currentBottom)
			};

			RECT textRect = cell;
			textRect.top = currentBottom - ((ySteps[j] + textHeights[j]) / 2);

			Rectangle(
				wndDC,
				cell.left,
				cell.top,
				cell.right + 1,
				cell.bottom + 1
			);

			::DrawText(wndDC, textToPrint, (int)wcslen(textToPrint),
				&textRect, DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL);
		}
	}

	// fill area to the right of the table
	::Rectangle(
		wndDC,
		(LONG)(xStep * columns),
		(LONG)0,
		(LONG)(xStep * columns + totalWidth % xStep + 1),
		(LONG)currentBottom + 1
	);
}

// initializes columnWidths_ & lineHeight_
void __stdcall SpreadSheet::getCellParameters(
	std::vector<std::vector<std::wstring>> strings
)

{
	HDC wndDC = ::GetDC(hWnd_);

	size_t totalColumns = 0;
	if (strings.size != 0)
	{
		totalColumns = strings[0].size();
	}

	columnWidths_ = std::vector<LONG>(totalColumns);

	SIZE stringSize = { };
	for (size_t j = 0; j < strings.size(); j++)
	{
		for (size_t i = 0; i < strings[j].size(); i++)
		{
			::GetTextExtentPoint32(wndDC, tableStrings_[i][j].c_str(),
				(int)strings[i].size(), &stringSize); // get string length 
			if (stringSize.cy > lineHeight_)
			{
				lineHeight_ = stringSize.cy;
			}
			if (stringSize.cx > columnWidths_[i])
			{
				columnWidths_[i] = stringSize.cx;
			}
		}		
	}

	::ReleaseDC(hWnd_, wndDC);
}





