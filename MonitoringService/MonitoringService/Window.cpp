#include "Window.h"

Window::Window(HINSTANCE hInstance, int nCmdShow)
{
	this->hInstance_ = hInstance;
	HWND hWnd = this->initialize(hInstance, nCmdShow);
	this->hWnd_ = hWnd;
	this->spreadSheet_ = new SpreadSheet(hWnd);
}

Window::~Window()
{
	// destroy child windows if they still exist
	if (editRowsHwnd_ != nullptr)
	{
		::DestroyWindow(editRowsHwnd_);
	}
	if (editColumnsHwnd_ != nullptr)
	{
		::DestroyWindow(editColumnsHwnd_);
	}
	if (buttonOkHwnd_ != nullptr)
	{
		::DestroyWindow(buttonOkHwnd_);
	}
	if (buttonLoadHwnd_ != nullptr)
	{
		::DestroyWindow(buttonLoadHwnd_);
	}

	if (spreadSheet_ != nullptr)
	{
		delete spreadSheet_;
	}
}

int Window::messageLoop()
{
	MSG msg;
	BOOL result;

	while (result = ::GetMessage(&msg, nullptr, 0, 0))
	{
		if (-1 == result)
		{
			// TODO: HANDLE THIS ERROR
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

HWND Window::initialize(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wndClassEx = { };
	this->registerClass(wndClassEx, hInstance, Window::windowProc);
	HWND hWnd = ::CreateWindow(DEFAULT_CLASS_NAME, DEFAULT_WINDOW_NAME, WS_OVERLAPPEDWINDOW,
		100, 100, DEFAULT_WINDOW_WIDTH_HEIGHT.x, DEFAULT_WINDOW_WIDTH_HEIGHT.y,
		HWND_DESKTOP, nullptr, hInstance, nullptr);

	// save a reference to the current Window instance 
	::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

	::ShowWindow(hWnd, nCmdShow);
	::UpdateWindow(hWnd);

	return hWnd;
}

ATOM Window::registerClass(WNDCLASSEX wndClassEx, HINSTANCE hInstance, WNDPROC wndProc)
{
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_DBLCLKS;
	wndClassEx.lpfnWndProc = wndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = hInstance;
	wndClassEx.hIcon = nullptr;
	wndClassEx.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wndClassEx.hbrBackground = ::CreateSolidBrush(DEFAULT_BACKGROUND_COLOR);
	wndClassEx.lpszMenuName = nullptr;
	wndClassEx.lpszClassName = DEFAULT_CLASS_NAME;
	wndClassEx.hIconSm = nullptr;
	return ::RegisterClassEx(&wndClassEx);
}

LRESULT CALLBACK Window::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// retrieve a reference to the related Window class
	Window* window = reinterpret_cast<Window*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

	SpreadSheet* s = nullptr;
	if (window != nullptr)
	{
		s = window->spreadSheet_;
	}

	switch (message)
	{
	case WM_CREATE:
	{
		::DefWindowProc(hWnd, message, wParam, lParam);
		::PostMessage(hWnd, WM_CREATE_CHILD_ITEMS, 0, 0);
		break;
	}
	case WM_CREATE_CHILD_ITEMS:
	{
		if (window != nullptr)
		{
			window->processCreateChildItemsRequest();
			bool result = window->loadStringsFromFile();
			if (!result)
			{
				::MessageBox(hWnd, L"Error while loading", L"Attention", MB_OK);
			}
		}
		break;
	}
	case WM_GETMINMAXINFO:
	{
		// set min table height & width 
		if ((s != nullptr) && s->isInitialized())
		{
			MINMAXINFO* m = (MINMAXINFO*)lParam;
			POINT minDimensions = s->getMinWindowSize();
			m->ptMinTrackSize.x = minDimensions.x;
			m->ptMinTrackSize.y = minDimensions.y;
		}
		return 0;
	}
	case WM_COMMAND:
	{
		if (window != nullptr)
		{
			// process click on OK button
			if (LOWORD(wParam) == window->DEFAULT_OK_NO)
			{
				if (window->loadedStrings_.size() != 0)
				{
					window->processShowSpreadsheetRequest(lParam);
				}
				else
				{
					::MessageBox(hWnd, L"File loading required", L"Attention", MB_OK);
				}
			}

			// process click on LOAD button
			if (LOWORD(wParam) == window->DEFAULT_LOAD_NO)
			{
				bool result = window->loadStringsFromFile();
				if (!result)
				{
					::MessageBox(hWnd, L"Error while loading", L"Attention", MB_OK);
				}
			}
		}
		break;
	}
	case WM_LOAD_SPREADSHEET:
	{
		if (s != nullptr)
		{
			s->update();
		}
		break;
	}
	case WM_SIZE:
	case WM_PAINT:
	{
		if (s != nullptr)
		{
			s->update();
		}
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_CLOSE:
	{
		::PostQuitMessage(0);
		break;
	}
	default:
	{
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
	}
	return 0;
}

bool Window::processShowSpreadsheetRequest(LPARAM lParam)
{
	bool result = false;

	SpreadSheet* s = this->spreadSheet_;
	HWND hWnd = this->hWnd_;

	WCHAR* rowsInput = (WCHAR*)::calloc(this->MAX_TEXT_LENGTH, sizeof(WCHAR));
	WCHAR* columnsInput = (WCHAR*)::calloc(this->MAX_TEXT_LENGTH, sizeof(WCHAR));

	// get input from window
	::GetWindowText(this->editRowsHwnd_, rowsInput, this->MAX_TEXT_LENGTH);
	::GetWindowText(this->editColumnsHwnd_, columnsInput, this->MAX_TEXT_LENGTH);

	int rows = ::_wtoi(rowsInput);
	int columns = ::_wtoi(columnsInput);
	if (s != nullptr)
	{
		bool wrongInput = (rows < s->MIN_LINES) || (rows > s->MAX_LINES) ||
			(columns < s->MIN_LINES) || (columns > s->MAX_LINES);
		if (wrongInput)
		{
			::MessageBox(hWnd, L"Incorrect table dimensions", L"Attention", MB_OK);
		}
		else // create table if input is correct
		{
			this->totalStrings_ = rows * columns;
			this->correctLoadedStrings();

			s->initialize(rows, columns, loadedStrings_, lParam);

			::DestroyWindow(this->editRowsHwnd_);
			::DestroyWindow(this->editColumnsHwnd_);
			::DestroyWindow(this->buttonOkHwnd_);
			::DestroyWindow(this->buttonLoadHwnd_);

			::PostMessage(this->hWnd_, WM_LOAD_SPREADSHEET, 0, 0);
			result = true;
		}
	}

	free(rowsInput);
	free(columnsInput);

	return result;
}

bool Window::processCreateChildItemsRequest()
{
	bool result =
		(this->editRowsHwnd_ == nullptr) &&
		(this->editColumnsHwnd_ == nullptr) &&
		(this->buttonLoadHwnd_ == nullptr) &&
		(this->buttonOkHwnd_ == nullptr);

	if (result)
	{
		// creating child windows 
		this->editRowsHwnd_ =
			::CreateWindow(L"EDIT", L"7",
				this->DEFAULT_CHILD_STYLE,
				this->EDIT_ROWS_DEFAULT_POSITION.left,
				this->EDIT_ROWS_DEFAULT_POSITION.top,
				this->EDIT_ROWS_DEFAULT_POSITION.right,
				this->EDIT_ROWS_DEFAULT_POSITION.bottom,
				this->hWnd_, nullptr, nullptr, nullptr
			);
		this->editColumnsHwnd_ =
			::CreateWindow(L"EDIT", L"5",
				this->DEFAULT_CHILD_STYLE,
				this->EDIT_COLUMNS_DEFAULT_POSITION.left,
				this->EDIT_COLUMNS_DEFAULT_POSITION.top,
				this->EDIT_COLUMNS_DEFAULT_POSITION.right,
				this->EDIT_COLUMNS_DEFAULT_POSITION.bottom,
				this->hWnd_, nullptr, nullptr, nullptr
			);
		this->buttonOkHwnd_ =
			::CreateWindow(L"BUTTON", L"GENERATE TABLE",
				this->DEFAULT_CHILD_STYLE,
				this->BUTTON_OK_DEFAULT_POSITION.left,
				this->BUTTON_OK_DEFAULT_POSITION.top,
				this->BUTTON_OK_DEFAULT_POSITION.right,
				this->BUTTON_OK_DEFAULT_POSITION.bottom,
				this->hWnd_, (HMENU)(this->DEFAULT_OK_NO), nullptr, nullptr
			);
		this->buttonLoadHwnd_ =
			::CreateWindow(L"BUTTON", L"LOAD FILE",
				this->DEFAULT_CHILD_STYLE,
				this->BUTTON_LOAD_DEFAULT_POSITION.left,
				this->BUTTON_LOAD_DEFAULT_POSITION.top,
				this->BUTTON_LOAD_DEFAULT_POSITION.right,
				this->BUTTON_LOAD_DEFAULT_POSITION.bottom,
				this->hWnd_, (HMENU)(this->DEFAULT_LOAD_NO), nullptr, nullptr
			);
	}

	return result;
}

bool Window::loadStringsFromFile()
{
	WCHAR fileName[MAX_PATH] = {};

	OPENFILENAME openFileName;
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = hWnd_;
	openFileName.hInstance = nullptr; /* Not supported */
	openFileName.lpstrFilter = L"Text files\0*.txt\0\0";
	openFileName.lpstrCustomFilter = nullptr;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFile = fileName;
	openFileName.nMaxFile = sizeof(fileName);
	openFileName.lpstrFileTitle = nullptr;
	openFileName.lpstrInitialDir = nullptr;
	openFileName.lpstrTitle = L"Select text file";
	openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	openFileName.lpstrDefExt = nullptr;

	bool sucessful = ::GetOpenFileName(&openFileName);

	if (sucessful)
	{
		loadedStrings_.clear();
		std::wifstream stream(fileName);

		// set file input format as utf-8
		stream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		std::wstring line;

		if (stream.is_open())
		{
			size_t maxCells = SpreadSheet::MAX_CELLS;
			for (size_t i = 0; i < maxCells; i++)
			{
				if (std::getline(stream, line))
				{
					loadedStrings_.push_back(line);
				}
				else
				{
					break;
				}
			}
			stream.close();
		}
		else
		{
			sucessful = false;
		}

		if (loadedStrings_.size() == 0)
		{
			sucessful = false;
		}
	}

	return sucessful;
}

bool Window::correctLoadedStrings()
{
	int linesToPush = this->totalStrings_ - (int)loadedStrings_.size();

	if (linesToPush > 0)
	{
		for (size_t i = 0; i < linesToPush; i++)
		{
			loadedStrings_.push_back(L"");
		}
	}
	else
	{
		int linesToPop = ::abs(linesToPush);
		for (size_t i = 0; i < linesToPop; i++)
		{
			loadedStrings_.pop_back();
		}
	}

	return true;
}