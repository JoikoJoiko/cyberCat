#include <windows.h>

constexpr COLORREF BG_COLOR = RGB(0, 0, 0);
constexpr COLORREF ACCENT_COLOR = RGB(57, 24, 157);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);

        HBRUSH bgBrush = CreateSolidBrush(BG_COLOR);
        FillRect(hdc, &rect, bgBrush);
        DeleteObject(bgBrush);

        HPEN pen = CreatePen(PS_SOLID, 2, ACCENT_COLOR);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

        MoveToEx(hdc, 0, 24, nullptr);
        LineTo(hdc, rect.right, 24);

        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        ReleaseCapture();
        SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return 0;
    }
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"CyberCatWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"cyberCat",
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1400, 800,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hwnd == nullptr)
        return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}