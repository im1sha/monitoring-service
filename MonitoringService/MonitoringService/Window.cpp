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
	if (spreadSheet_ != nullptr)
	{
		delete spreadSheet_;
	}
}

int __stdcall Window::messageLoop()
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

HWND __stdcall Window::initialize(HINSTANCE hInstance, int nCmdShow)
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

ATOM __stdcall Window::registerClass(WNDCLASSEX wndClassEx, HINSTANCE hInstance, WNDPROC wndProc)
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

LRESULT __stdcall Window::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
		::PostMessage(hWnd, WM_CREATE_SPREADSHEET, 0, 0);
		break;
	}
	case WM_CREATE_SPREADSHEET:
	{
		if (window != nullptr && window->spreadSheet_ != nullptr)
		{
			window->processCreationRequest();
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
	case WM_TIMER:
	{
		if (s != nullptr)
		{
			s->respondOnTimer();
		}
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

bool __stdcall Window::processCreationRequest()
{
	SpreadSheet* s = this->spreadSheet_;
	HWND hWnd = this->hWnd_;

	s->initialize();
	::PostMessage(this->hWnd_, WM_LOAD_SPREADSHEET, 0, 0);

	return true;
}


