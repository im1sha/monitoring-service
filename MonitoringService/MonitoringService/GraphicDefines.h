#pragma once

#define WM_CREATE_SPREADSHEET (WM_USER)
#define WM_LOAD_SPREADSHEET (WM_USER + 1)
#define WM_LOAD_SPREADSHEET_LEFT (WM_USER + 2)
#define WM_LOAD_SPREADSHEET_RIGHT (WM_USER + 3)
#define WM_LOAD_SPREADSHEET_UP (WM_USER + 4)
#define WM_LOAD_SPREADSHEET_DOWN (WM_USER + 5)


#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

#define DEFAULT_BACKGROUND_COLOR GetSysColor(COLOR_3DDKSHADOW)