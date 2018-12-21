#pragma once

#include "SpreadSheet.h"

SpreadSheet::SpreadSheet(HWND hWnd, ProcessMonitor* monitor)
{
	piSection_ = new CRITICAL_SECTION();
	::InitializeCriticalSectionAndSpinCount(piSection_, DEFAULT_SPIN_COUNT);
	this->hWnd_ = hWnd;
	this->monitor_ = monitor;
	if (this->monitor_ != nullptr)
	{
		this->monitor_->runAsBackground();
	}
	::SetTimer(hWnd, 1, 2000, nullptr);
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
	::KillTimer(hWnd_, 1);
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

	if (piSection_ != nullptr)
	{
		::DeleteCriticalSection(piSection_);
		delete piSection_;
	}
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
	::EnterCriticalSection(piSection_);
	this->pi_ = this->monitor_->getPerfomanceData();
	::LeaveCriticalSection(piSection_);
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
	if (isInitialized())
	{
		::EnterCriticalSection(piSection_);
		std::vector<std::vector<std::wstring>> content = processInfoVectorToWstrVector(pi_);
		::LeaveCriticalSection(piSection_);
		if (content.size() != 0)
		{
			this->draw(content);
		}
	}
}

// draws table and fill it with content
void __stdcall SpreadSheet::draw(std::vector<std::vector<std::wstring>> content)
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

	// width & height calculating
	this->getCellParameters(content);

	// table painting
	if (content.size() == 0)
	{
		return;
	}
	this->paintTable(lineHeight_, columnWidths_, 
		clientRect.right - clientRect.left, 
		content, wndDC);

	// draw ending
	::EndPaint(hWnd_, &ps);
	::ReleaseDC(hWnd_, wndDC);
}

// paints table 
void __stdcall SpreadSheet::paintTable(
	LONG yStep,
	std::vector<LONG> xSteps,
	LONG totalWidth, 
	std::vector<std::vector<std::wstring>> strings, 
	HDC wndDC
)
{
	size_t rows = strings.size();
	size_t columns = strings[0].size();

	LONG currentBottom = 0;

	for (size_t j = 0; j < rows; j++)
	{
		currentBottom += yStep;
		LONG currentRigthBound = 0;

		for (size_t i = 0; i < columns; i++)
		{
			currentRigthBound += xSteps[i];

			const WCHAR* textToPrint = strings[j][i].c_str();

			RECT cell = {
				currentRigthBound - xSteps[i],
				currentBottom - yStep,
				currentRigthBound,
				currentBottom
			};

			RECT textRect = cell;
			//textRect.top = currentBottom - ((xSteps[j] + yStep) / 2);

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
		(LONG)(yStep * columns),
		(LONG)0,
		(LONG)(yStep * columns + totalWidth % yStep + 1),
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
	if (strings.size() != 0)
	{
		totalColumns = strings[0].size();
	}

	lineHeight_ = 0;
	columnWidths_ = std::vector<LONG>(totalColumns);

	SIZE stringSize = { };
	for (size_t i = 0; i < strings.size(); i++) // 
	{
		for (size_t j = 0; j < strings[i].size(); j++)
		{
			const WCHAR* str = strings[i][j].c_str();

			::GetTextExtentPoint32(wndDC, str,
				(int)strings[i][j].length() + 1, &stringSize); // get string length 
			
			
			if (stringSize.cy > lineHeight_)
			{
				lineHeight_ = stringSize.cy;
			}
			if (stringSize.cx > columnWidths_[j])
			{
				columnWidths_[j] = stringSize.cx;
			}
		}		
	}

	::ReleaseDC(hWnd_, wndDC);
}

std::wstring __stdcall SpreadSheet::doubleToWstring(double d) 
{
	std::wstringstream s;
	s << d;
	std::wstring wstr(s.str());
	return wstr;
}

std::wstring __stdcall SpreadSheet::dwordToWstring(DWORD d)
{
	std::wstringstream s;
	s << d;
	std::wstring wstr(s.str());
	return wstr;
}

std::vector<std::vector<std::wstring>> __stdcall SpreadSheet::processInfoVectorToWstrVector(
	std::vector<ProcessInfo> pi
) 
{
	std::vector<std::vector<std::wstring>> result;

	for (size_t i = 0; i < pi.size(); i++)
	{
		result.push_back(std::vector<std::wstring>());
	}

	for (size_t i = 0; i < pi.size(); i++)
	{
		result[i].push_back(dwordToWstring((pi)[i].processId));
		result[i].push_back(dwordToWstring((pi)[i].runThreads));
		result[i].push_back(dwordToWstring((pi)[i].parentProcessId));

		result[i].push_back(std::wstring((pi)[i].fileName));
		result[i].push_back(std::wstring((pi)[i].userName));
		result[i].push_back(std::wstring((pi)[i].domainName));

		result[i].push_back(doubleToWstring((pi)[i].workingSetInMb));
		result[i].push_back(doubleToWstring((pi)[i].workingSetPrivateInMb));
		result[i].push_back(doubleToWstring((pi)[i].io));
		result[i].push_back(doubleToWstring((pi)[i].processorUsage));
		result[i].push_back(doubleToWstring((pi)[i].elapsedTime));
	}

	return result;
}



