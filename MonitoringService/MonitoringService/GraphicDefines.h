#pragma once

#define WM_CREATE_CHILD_ITEMS (WM_USER)
#define WM_LOAD_SPREADSHEET (WM_USER + 1)

#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

#define DEFAULT_BACKGROUND_COLOR GetSysColor(COLOR_3DDKSHADOW)