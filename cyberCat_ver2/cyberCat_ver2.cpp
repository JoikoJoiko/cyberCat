#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <strsafe.h>
#include <ctime>

#include "app_config.h"

#pragma comment(lib, "dwmapi.lib")

constexpr COLORREF BG_COLOR = RGB(0, 0, 0);
constexpr COLORREF ACCENT_COLOR = RGB(57, 24, 157);
constexpr COLORREF TITLEBAR_COLOR = RGB(8, 8, 8);
constexpr COLORREF HOVER_COLOR = RGB(20, 10, 55);

constexpr int WINDOW_WIDTH = 1400;
constexpr int WINDOW_HEIGHT = 800;
constexpr int TITLEBAR_HEIGHT = 28;
constexpr int BUTTON_WIDTH = 46;
constexpr int CORNER_RADIUS = 12;
constexpr int RESIZE_BORDER = 6;
constexpr int SIDE_MENU_WIDTH = 160;
constexpr int MENU_ITEM_HEIGHT = 86;
constexpr int WM_APP_CLOCKTICK = 0x5001;

enum class CaptionButton
{
    None,
    Minimize,
    Maximize,
    Close
};

struct UiState
{
    CaptionButton hoveredButton = CaptionButton::None;
    bool isMenuOpen = true;
};

struct WindowState
{
    bool isMaximized = false;
    RECT restoreRect{};
};

UiState g_uiState;
WindowState g_windowState;

RECT GetMinButtonRect(const RECT& rc)
{
    return RECT{ rc.right - BUTTON_WIDTH * 3, 0, rc.right - BUTTON_WIDTH * 2, TITLEBAR_HEIGHT };
}

RECT GetMaxButtonRect(const RECT& rc)
{
    return RECT{ rc.right - BUTTON_WIDTH * 2, 0, rc.right - BUTTON_WIDTH, TITLEBAR_HEIGHT };
}

RECT GetCloseButtonRect(const RECT& rc)
{
    return RECT{ rc.right - BUTTON_WIDTH, 0, rc.right, TITLEBAR_HEIGHT };
}

RECT GetMenuToggleRect()
{
    return RECT{ 10, 0, 140, TITLEBAR_HEIGHT };
}

RECT GetSideMenuRect(const RECT& rcClient)
{
    return RECT{ 0, TITLEBAR_HEIGHT, SIDE_MENU_WIDTH, rcClient.bottom };
}

bool PointInRectEx(POINT pt, const RECT& rc)
{
    return pt.x >= rc.left && pt.x < rc.right && pt.y >= rc.top && pt.y < rc.bottom;
}

CaptionButton HitTestCaptionButton(HWND hwnd, POINT pt)
{
    RECT rcClient{};
    GetClientRect(hwnd, &rcClient);

    RECT rcMin = GetMinButtonRect(rcClient);
    RECT rcMax = GetMaxButtonRect(rcClient);
    RECT rcClose = GetCloseButtonRect(rcClient);

    if (PointInRectEx(pt, rcClose)) return CaptionButton::Close;
    if (PointInRectEx(pt, rcMax)) return CaptionButton::Maximize;
    if (PointInRectEx(pt, rcMin)) return CaptionButton::Minimize;

    return CaptionButton::None;
}

void FillRectColor(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void DrawButtonText(HDC hdc, const RECT& rc, const wchar_t* text)
{
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, ACCENT_COLOR);
    DrawTextW(hdc, text, -1, const_cast<RECT*>(&rc), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void ApplyRoundedCorners(HWND hwnd)
{
    RECT rc{};
    GetWindowRect(hwnd, &rc);

    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    if (width <= 0 || height <= 0)
        return;

    HRGN hRgn = CreateRoundRectRgn(
        0, 0,
        width + 1, height + 1,
        CORNER_RADIUS, CORNER_RADIUS
    );

    SetWindowRgn(hwnd, hRgn, TRUE);

    const DWORD dwmCornerPref = 2;
    DwmSetWindowAttribute(hwnd, 33, &dwmCornerPref, sizeof(dwmCornerPref));

    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));

    COLORREF borderColor = RGB(0, 0, 0);
    DwmSetWindowAttribute(hwnd, 34, &borderColor, sizeof(borderColor));
}

void ToggleMaximize(HWND hwnd)
{
    if (!g_windowState.isMaximized)
    {
        GetWindowRect(hwnd, &g_windowState.restoreRect);

        HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{};
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(hMonitor, &mi);

        const RECT& rc = mi.rcWork;

        SetWindowPos(
            hwnd,
            nullptr,
            rc.left,
            rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            SWP_NOZORDER | SWP_NOACTIVATE
        );

        g_windowState.isMaximized = true;
    }
    else
    {
        const RECT& rc = g_windowState.restoreRect;

        SetWindowPos(
            hwnd,
            nullptr,
            rc.left,
            rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            SWP_NOZORDER | SWP_NOACTIVATE
        );

        g_windowState.isMaximized = false;
    }

    ApplyRoundedCorners(hwnd);
    InvalidateRect(hwnd, nullptr, FALSE);
}

void FormatClockText(wchar_t* buffer, size_t bufferCount)
{
    std::time_t now = std::time(nullptr);
    now += static_cast<std::time_t>(AppConfig::kTimezoneOffsetHours) * 60 * 60;

    std::tm utcTm{};
    gmtime_s(&utcTm, &now);

    wchar_t sign = AppConfig::kTimezoneOffsetHours >= 0 ? L'+' : L'-';
    int offset = AppConfig::kTimezoneOffsetHours >= 0 ? AppConfig::kTimezoneOffsetHours : -AppConfig::kTimezoneOffsetHours;

    StringCchPrintfW(
        buffer,
        bufferCount,
        L"%02d/%02d/%04d %02d:%02d:%02d GMT%c%d",
        utcTm.tm_mday,
        utcTm.tm_mon + 1,
        utcTm.tm_year + 1900,
        utcTm.tm_hour,
        utcTm.tm_min,
        utcTm.tm_sec,
        sign,
        offset
    );
}

void DrawMenuToggle(HDC hdc)
{
    RECT rcMenu = GetMenuToggleRect();

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, ACCENT_COLOR);

    RECT rcIcon{ rcMenu.left, rcMenu.top, rcMenu.left + 24, rcMenu.bottom };
    DrawTextW(hdc, L"◈", -1, &rcIcon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    RECT rcText{ rcMenu.left + 24, rcMenu.top, rcMenu.right, rcMenu.bottom };
    DrawTextW(hdc, AppConfig::kMenuLabel, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void DrawSideMenu(HDC hdc, const RECT& rcClient)
{
    RECT rcSide = GetSideMenuRect(rcClient);
    FillRectColor(hdc, rcSide, TITLEBAR_COLOR);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, ACCENT_COLOR);

    for (int i = 0; i < AppConfig::kMenuItemCount; ++i)
    {
        RECT rcItem{
            rcSide.left,
            TITLEBAR_HEIGHT + i * MENU_ITEM_HEIGHT,
            rcSide.right,
            TITLEBAR_HEIGHT + (i + 1) * MENU_ITEM_HEIGHT
        };

        RECT rcIcon{ rcItem.left + 18, rcItem.top, rcItem.left + 40, rcItem.bottom };
        const wchar_t* icon = L"◌";
        if (i == 0) icon = L"⚙";
        if (i == 1) icon = L"☷";
        if (i == 2) icon = L"◴";

        DrawTextW(hdc, icon, -1, &rcIcon, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        RECT rcLabel{ rcItem.left + 48, rcItem.top, rcItem.right - 8, rcItem.bottom };
        DrawTextW(hdc, AppConfig::kMenuItems[i], -1, &rcLabel, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    RECT rcVersion{ rcSide.left + 18, rcClient.bottom - 36, rcSide.right - 8, rcClient.bottom - 10 };
    DrawTextW(hdc, AppConfig::kVersionLabel, -1, &rcVersion, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void DrawWelcomeText(HDC hdc, const RECT& rcClient)
{
    RECT rcWelcome = rcClient;
    rcWelcome.top = TITLEBAR_HEIGHT;

    if (g_uiState.isMenuOpen)
    {
        rcWelcome.left = SIDE_MENU_WIDTH;
    }

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, ACCENT_COLOR);

    RECT rcLine1{ rcWelcome.right - 470, rcWelcome.top + 300, rcWelcome.right - 70, rcWelcome.top + 380 };
    RECT rcLine2{ rcWelcome.right - 420, rcWelcome.top + 360, rcWelcome.right - 70, rcWelcome.top + 450 };

    DrawTextW(hdc, AppConfig::kWelcomeLine1, -1, &rcLine1, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hdc, AppConfig::kWelcomeLine2, -1, &rcLine2, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void DrawTitleBar(HDC hdc, HWND hwnd)
{
    RECT rcClient{};
    GetClientRect(hwnd, &rcClient);

    RECT rcTitleBar{ 0, 0, rcClient.right, TITLEBAR_HEIGHT };
    FillRectColor(hdc, rcTitleBar, TITLEBAR_COLOR);

    RECT rcMin = GetMinButtonRect(rcClient);
    RECT rcMax = GetMaxButtonRect(rcClient);
    RECT rcClose = GetCloseButtonRect(rcClient);

    FillRectColor(hdc, rcMin, g_uiState.hoveredButton == CaptionButton::Minimize ? HOVER_COLOR : TITLEBAR_COLOR);
    FillRectColor(hdc, rcMax, g_uiState.hoveredButton == CaptionButton::Maximize ? HOVER_COLOR : TITLEBAR_COLOR);
    FillRectColor(hdc, rcClose, g_uiState.hoveredButton == CaptionButton::Close ? HOVER_COLOR : TITLEBAR_COLOR);

    DrawMenuToggle(hdc);

    wchar_t clockBuffer[64]{};
    FormatClockText(clockBuffer, _countof(clockBuffer));

    RECT rcClock{ 0, 0, rcClient.right, TITLEBAR_HEIGHT };
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, ACCENT_COLOR);
    DrawTextW(hdc, clockBuffer, -1, &rcClock, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DrawButtonText(hdc, rcMin, L"—");
    DrawButtonText(hdc, rcMax, g_windowState.isMaximized ? L"❐" : L"□");
    DrawButtonText(hdc, rcClose, L"×");

    HPEN pen = CreatePen(PS_SOLID, 1, ACCENT_COLOR);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    MoveToEx(hdc, 0, TITLEBAR_HEIGHT - 1, nullptr);
    LineTo(hdc, rcClient.right, TITLEBAR_HEIGHT - 1);

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

LRESULT HandleNcHitTest(HWND hwnd, LPARAM lParam)
{
    if (g_windowState.isMaximized)
        return HTCLIENT;

    POINT screenPt{
        GET_X_LPARAM(lParam),
        GET_Y_LPARAM(lParam)
    };

    RECT winRect{};
    GetWindowRect(hwnd, &winRect);

    const bool onLeft = screenPt.x >= winRect.left && screenPt.x < winRect.left + RESIZE_BORDER;
    const bool onRight = screenPt.x < winRect.right && screenPt.x >= winRect.right - RESIZE_BORDER;
    const bool onTop = screenPt.y >= winRect.top && screenPt.y < winRect.top + RESIZE_BORDER;
    const bool onBottom = screenPt.y < winRect.bottom && screenPt.y >= winRect.bottom - RESIZE_BORDER;

    if (onTop && onLeft) return HTTOPLEFT;
    if (onTop && onRight) return HTTOPRIGHT;
    if (onBottom && onLeft) return HTBOTTOMLEFT;
    if (onBottom && onRight) return HTBOTTOMRIGHT;
    if (onLeft) return HTLEFT;
    if (onRight) return HTRIGHT;
    if (onTop) return HTTOP;
    if (onBottom) return HTBOTTOM;

    POINT clientPt = screenPt;
    ScreenToClient(hwnd, &clientPt);

    if (clientPt.y >= 0 && clientPt.y < TITLEBAR_HEIGHT)
    {
        if (HitTestCaptionButton(hwnd, clientPt) == CaptionButton::None)
            return HTCAPTION;
    }

    return HTCLIENT;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        ApplyRoundedCorners(hwnd);
        SetTimer(hwnd, WM_APP_CLOCKTICK, 1000, nullptr);
        return 0;

    case WM_SIZE:
        ApplyRoundedCorners(hwnd);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_TIMER:
        if (wParam == WM_APP_CLOCKTICK)
        {
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        break;

    case WM_NCHITTEST:
        return HandleNcHitTest(hwnd, lParam);

    case WM_MOUSEMOVE:
    {
        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        CaptionButton newHovered = HitTestCaptionButton(hwnd, pt);

        if (newHovered != g_uiState.hoveredButton)
        {
            g_uiState.hoveredButton = newHovered;
            InvalidateRect(hwnd, nullptr, FALSE);

            TRACKMOUSEEVENT tme{};
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
        }
        return 0;
    }

    case WM_MOUSELEAVE:
        g_uiState.hoveredButton = CaptionButton::None;
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_LBUTTONDOWN:
    {
        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        CaptionButton hit = HitTestCaptionButton(hwnd, pt);

        if (hit == CaptionButton::Minimize)
        {
            ShowWindow(hwnd, SW_MINIMIZE);
            return 0;
        }

        if (hit == CaptionButton::Maximize)
        {
            ToggleMaximize(hwnd);
            return 0;
        }

        if (hit == CaptionButton::Close)
        {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }

        if (PointInRectEx(pt, GetMenuToggleRect()))
        {
            g_uiState.isMenuOpen = !g_uiState.isMenuOpen;
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        return 0;
    }

    case WM_LBUTTONDBLCLK:
    {
        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (pt.y >= 0 && pt.y < TITLEBAR_HEIGHT &&
            HitTestCaptionButton(hwnd, pt) == CaptionButton::None &&
            !PointInRectEx(pt, GetMenuToggleRect()))
        {
            ToggleMaximize(hwnd);
            return 0;
        }
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect{};
        GetClientRect(hwnd, &rect);

        FillRectColor(hdc, rect, BG_COLOR);
        DrawTitleBar(hdc, hwnd);

        if (g_uiState.isMenuOpen)
        {
            DrawSideMenu(hdc, rect);
        }

        DrawWelcomeText(hdc, rect);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        KillTimer(hwnd, WM_APP_CLOCKTICK);
        PostQuitMessage(0);
        return 0;

    case WM_NCCALCSIZE:
        if (wParam == TRUE)
            return 0;
        break;
    }


    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"CyberCatWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        CLASS_NAME,
        AppConfig::kAppTitle,
        WS_POPUP | WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hwnd == nullptr)
        return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
