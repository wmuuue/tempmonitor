#include "FloatingWindow.h"
#include <windowsx.h>
#include <gdiplus.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

FloatingWindow::FloatingWindow(Config* cfg, TempMonitor* mon)
    : hwnd(nullptr), config(cfg), monitor(mon), visible(false), dragging(false) {
    currentData = {};
    currentWarningTemp = 70;
    currentDangerTemp = 85;
}

FloatingWindow::~FloatingWindow() {
    if (hwnd) {
        DestroyWindow(hwnd);
    }
}

bool FloatingWindow::Create(HINSTANCE hInstance) {
    const wchar_t CLASS_NAME[] = L"TempMonitorFloatingWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);

    int x = config->GetWindowX();
    int y = config->GetWindowY();
    
    if (x == -1 || y == -1) {
        x = (GetSystemMetrics(SM_CXSCREEN) - 300) / 2;
        y = (GetSystemMetrics(SM_CYSCREEN) - 150) / 2;
    }

    hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        L"Temperature Monitor",
        WS_POPUP,
        x, y, 300, 150,
        NULL, NULL, hInstance, this
    );

    if (!hwnd) return false;

    SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 128, LWA_ALPHA);

    return true;
}

void FloatingWindow::Show() {
    if (hwnd && !visible) {
        ShowWindow(hwnd, SW_SHOW);
        visible = true;
    }
}

void FloatingWindow::Hide() {
    if (hwnd && visible) {
        ShowWindow(hwnd, SW_HIDE);
        visible = false;
        SavePosition();
    }
}

void FloatingWindow::UpdateTemp(const TempData& data, int warningTemp, int dangerTemp) {
    currentData = data;
    currentWarningTemp = warningTemp;
    currentDangerTemp = dangerTemp;
    
    if (hwnd) {
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void FloatingWindow::SavePosition() {
    if (hwnd) {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        config->SetWindowX(rect.left);
        config->SetWindowY(rect.top);
        config->Save();
    }
}

LRESULT CALLBACK FloatingWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    FloatingWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (FloatingWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = (FloatingWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        switch (uMsg) {
        case WM_PAINT:
            pThis->OnPaint();
            return 0;
        case WM_LBUTTONDOWN:
            pThis->OnMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_MOUSEMOVE:
            pThis->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_LBUTTONUP:
            pThis->OnMouseUp();
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void FloatingWindow::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    RECT rect;
    GetClientRect(hwnd, &rect);

    // Determine color based on temperature level
    TempLevel maxLevel = TempLevel::Normal;
    if (currentData.cpuTemp >= currentDangerTemp || currentData.gpuTemp >= currentDangerTemp) {
        maxLevel = TempLevel::Danger;
    } else if (currentData.cpuTemp >= currentWarningTemp || currentData.gpuTemp >= currentWarningTemp) {
        maxLevel = TempLevel::Warning;
    }

    Color bgColor = GetColorForLevel(maxLevel);
    SolidBrush brush(bgColor);
    graphics.FillEllipse(&brush, 0, 0, rect.right, rect.bottom);

    // Draw text
    FontFamily fontFamily(L"Segoe UI");
    Font font(&fontFamily, 14, FontStyleBold, UnitPixel);
    SolidBrush textBrush(Color(255, 255, 255, 255));

    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);

    std::wstringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << L"CPU: " << currentData.cpuTemp << L"°C\n";
    ss << L"GPU: " << currentData.gpuTemp << L"°C";
    if (currentData.fanSpeed > 0) {
        ss << L"\nFan: " << currentData.fanSpeed << L"%";
    }

    RectF layoutRect(0, 0, (REAL)rect.right, (REAL)rect.bottom);
    graphics.DrawString(ss.str().c_str(), -1, &font, layoutRect, &format, &textBrush);

    EndPaint(hwnd, &ps);
}

void FloatingWindow::OnMouseDown(int x, int y) {
    dragging = true;
    dragOffset.x = x;
    dragOffset.y = y;
    SetCapture(hwnd);
}

void FloatingWindow::OnMouseMove(int x, int y) {
    if (dragging) {
        POINT pt;
        GetCursorPos(&pt);
        SetWindowPos(hwnd, NULL, 
            pt.x - dragOffset.x, pt.y - dragOffset.y, 
            0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void FloatingWindow::OnMouseUp() {
    if (dragging) {
        dragging = false;
        ReleaseCapture();
        SavePosition();
    }
}

Color FloatingWindow::GetColorForLevel(TempLevel level) {
    switch (level) {
    case TempLevel::Danger:
        return Color(128, 255, 100, 150);  // Red with 50% alpha
    case TempLevel::Warning:
        return Color(128, 255, 200, 100);  // Yellow with 50% alpha
    case TempLevel::Normal:
    default:
        return Color(128, 255, 192, 203);  // Pink with 50% alpha
    }
}
