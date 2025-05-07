#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN

#include <winsdkver.h>
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <SDKDDKVer.h>

#include <windows.h>
#include <wingdi.h>
#include <strsafe.h>
#include <math.h>

#define IDT_TIMER1 1030
#define IDM_ALWAYS  101
#define IDM_SECONDS 102

#include "resource.h"

// which format of time to use
#define CRAZY_MODE

#ifdef CRAZY_MODE
#define TIMER_INTERVAL 2250
#else
#define TIMER_INTERVAL 1000
#endif


// Global Variables:
WCHAR szTitle[] = L"Clock";                  // The title bar text
WCHAR szWindowClass[] = L"CLOCK";            // the main window class name
BOOL g_topMost = FALSE;
BOOL g_seconds = TRUE;

// Forward declarations of functions included in this code module:
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{

    WNDCLASSEXW wcex;
    MSG msg;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    wcex.hIcon = NULL;
    wcex.hIconSm = NULL;

    RegisterClassExW(&wcex);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// position constants
#define WINDOWSIZE 150
#define CLOCK_CTR (WINDOWSIZE / 2)
#define CLOCK_MH  (CLOCK_CTR - 18)
#define CLOCK_HH  (CLOCK_MH * 70 / 100)

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   DWORD dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
   RECT rWnd = { 0, 0, WINDOWSIZE, WINDOWSIZE }, rDesktop;

   GetWindowRect(GetDesktopWindow, &rDesktop);
   AdjustWindowRectEx(&rWnd, dwStyle, FALSE, WS_EX_PALETTEWINDOW | WS_EX_APPWINDOW);

   hWnd = CreateWindowExW(WS_EX_PALETTEWINDOW | WS_EX_APPWINDOW, szWindowClass, szTitle, dwStyle,
      CW_USEDEFAULT, 0, rWnd.right - rWnd.left, rWnd.bottom - rWnd.top, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


void DrawTime(HDC hdc)
{
    RECT rect = { 0, 0, CLOCK_CTR * 2, 50 };
    WCHAR pszBuf[12], wchSign;
    SYSTEMTIME st;
    INT iHr, iMin, iSec, iMsCalc;
    double dx, dy, da;

    GetLocalTime(&st);
#ifdef CRAZY_MODE
    iHr = (st.wHour + 4) % 24;
    iMsCalc = st.wMilliseconds + (st.wSecond + st.wMinute * 60) * 1000;
    iMsCalc = iMsCalc * 40 / 60 * 40 / 60 / 1000;
    iMin = iMsCalc / 40;
    iSec = iMsCalc % 40;
#else
    iHr = st.wHour;
    iSec = st.wSecond;
    iMin = st.wMinute;
#endif

    // lines
    // There used to be one on top too, but the time is in the way
    MoveToEx(hdc, CLOCK_CTR, 2 * CLOCK_CTR - 2, NULL);
    LineTo(hdc, CLOCK_CTR, 2 * CLOCK_CTR - 10);
    MoveToEx(hdc, 2 * CLOCK_CTR - 2, CLOCK_CTR, NULL);
    LineTo(hdc, 2 * CLOCK_CTR - 10, CLOCK_CTR);
    MoveToEx(hdc, 2, CLOCK_CTR, NULL);
    LineTo(hdc, 10, CLOCK_CTR);

#ifdef CRAZY_MODE
    // Since we use 8 hours we might as well mark all 8
    MoveToEx(hdc, 2, 2, NULL);
    LineTo(hdc, 10, 10);
    MoveToEx(hdc, CLOCK_CTR * 2 - 2, 2, NULL);
    LineTo(hdc, CLOCK_CTR * 2 - 10, 10);
    MoveToEx(hdc, 2, CLOCK_CTR * 2 - 2, NULL);
    LineTo(hdc, 10, CLOCK_CTR * 2 - 10);
    MoveToEx(hdc, CLOCK_CTR * 2 - 2, CLOCK_CTR * 2 - 2, NULL);
    LineTo(hdc, CLOCK_CTR * 2 - 10, CLOCK_CTR * 2 - 10);
#endif

    // text
#ifdef CRAZY_MODE
    wchSign = iHr < 9 ? L'Z' : iHr < 17 ? L'A' : L'P';
    iHr %= 8;
#else
    wchSign = iHr < 13 ? L'A' : L'P';
    iHr %= 12;
#endif
    StringCchPrintf(pszBuf, 12, L"%02d:%02d:%02d %cM", iHr, iMin, iSec, wchSign);
    DrawText(hdc, pszBuf, lstrlen(pszBuf), &rect, DT_CENTER | DT_NOCLIP | DT_SINGLELINE);

#ifdef CRAZY_MODE
#define CLOCK_FRACTION 40.0
#else
#define CLOCK_FRACTION 60.0
#endif

    // minute hand
    MoveToEx(hdc, CLOCK_CTR, CLOCK_CTR, NULL);
    da = (M_PI * 2 * iMin / CLOCK_FRACTION) - (M_PI / 2);
    dx = cos(da) * CLOCK_MH;
    dy = sin(da) * CLOCK_MH;
    LineTo(hdc, CLOCK_CTR + (int)dx, CLOCK_CTR + (int)dy);

    // hour hand
    MoveToEx(hdc, CLOCK_CTR, CLOCK_CTR, NULL);
    da = ((M_PI * 2) * ((iHr * 5 + (iMin / CLOCK_FRACTION) * 5) / CLOCK_FRACTION)) - (M_PI / 2);
    dx = cos(da) * CLOCK_HH;
    dy = sin(da) * CLOCK_HH;
    LineTo(hdc, CLOCK_CTR + (int)dx, CLOCK_CTR + (int)dy);

    // second hand
    SetDCPenColor(hdc, RGB(255, 0, 0));
    SelectObject(hdc, GetStockObject(DC_PEN));
    MoveToEx(hdc, CLOCK_CTR, CLOCK_CTR, NULL);
    da = (M_PI * 2 * iSec / CLOCK_FRACTION) - (M_PI / 2);
    dy = sin(da) * CLOCK_MH;
    LineTo(hdc, CLOCK_CTR + (int)(cos(da) * CLOCK_MH), CLOCK_CTR + (int)dy);
    SetDCPenColor(hdc, RGB(0, 0, 0));
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, IDT_TIMER1, TIMER_INTERVAL, NULL);
        {
            HMENU hMenu = GetSystemMenu(hWnd, FALSE);
            InsertMenu(hMenu, 0, MF_BYPOSITION, IDM_ALWAYS, L"&Always on top");
            InsertMenu(hMenu, 1, MF_BYPOSITION | MF_DISABLED, IDM_SECONDS, L"&Show seconds");
            InsertMenu(hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, -1, 0);
        }
        break;
    case WM_TIMER:
    case WM_TIMECHANGE:
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
        break;
    case WM_SYSCOMMAND:
        switch (LOWORD(wParam)) {
        case IDM_ALWAYS:
            if (g_topMost) {
                SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                CheckMenuItem(GetSystemMenu(hWnd, FALSE), IDM_ALWAYS, MF_UNCHECKED);
                g_topMost = 0;
            } else {
                SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                CheckMenuItem(GetSystemMenu(hWnd, FALSE), IDM_ALWAYS, MF_CHECKED);
                g_topMost = 1;
            }
            break;
        case IDM_SECONDS:
            if (g_seconds) {
                SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                CheckMenuItem(GetSystemMenu(hWnd, FALSE), IDM_SECONDS, MF_UNCHECKED);
                g_seconds = 0;
            }
            else {
                SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                CheckMenuItem(GetSystemMenu(hWnd, FALSE), IDM_SECONDS, MF_CHECKED);
                g_seconds = 1;
            }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            DrawTime(hdc);

            EndPaint(hWnd, &ps);
        }
        break;
        // TODO: pause/resume timer on deactivate?
    case WM_DESTROY:
        KillTimer(hWnd, IDT_TIMER1);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
