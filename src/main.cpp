#include <windows.h>
#include <shellapi.h>
#include <string>
#include <tchar.h>
#include <sstream>
#include <iomanip>
#include "TempMonitor.hpp"

// IDs
#define ID_TRAY_APP_ICON    1001
#define ID_TRAY_EXIT        1002
#define ID_TRAY_SETTINGS    1003
#define ID_TRAY_AUTOSTART   1004
#define WM_TRAYICON         (WM_USER + 1)
#define IDT_TIMER_UPDATE    1005

// Globals
HINSTANCE hInst;
HWND hOverlayWnd = NULL;
HWND hMainWnd = NULL;
NOTIFYICONDATA nid;
TempMonitor monitor;
double cpuTemp = 0.0;
double gpuTemp = 0.0;
double thresholdTemp = 80.0; // Default threshold
bool isOverlayVisible = false;
bool isAutoStart = false;
POINT lastOverlayPos = { 100, 100 };

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK OverlayWndProc(HWND, UINT, WPARAM, LPARAM);
void ShowOverlay(bool show);
void UpdateTrayTooltip();
void SetStartup(bool enable);
bool CheckStartup();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Suppress unreferenced parameter warnings
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    hInst = hInstance;

    // 1. Initialize Monitor
    if (!monitor.initialize()) {
        MessageBox(NULL, _T("Failed to initialize Monitor (WMI/NVML)"), _T("Warning"), MB_ICONWARNING);
    }

    // 2. Check AutoStart Status
    isAutoStart = CheckStartup();

    // 3. Register Classes
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, LoadIcon(hInstance, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL, _T("TempMonitorMainClass"), LoadIcon(hInstance, IDI_APPLICATION) };
    RegisterClassEx(&wcex);

    WNDCLASSEX wcexOverlay = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, OverlayWndProc, 0, 0, hInstance, NULL, LoadCursor(NULL, IDC_HAND), CreateSolidBrush(RGB(255, 192, 203)), NULL, _T("TempMonitorOverlayClass"), NULL };
    RegisterClassEx(&wcexOverlay);

    // 4. Create Main Window
    hMainWnd = CreateWindow(_T("TempMonitorMainClass"), _T("Temp Monitor"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (!hMainWnd) return FALSE;

    // 5. Create Overlay Window
    hOverlayWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, _T("TempMonitorOverlayClass"), _T(""), WS_POPUP, lastOverlayPos.x, lastOverlayPos.y, 200, 100, NULL, NULL, hInstance, NULL);
    SetLayeredWindowAttributes(hOverlayWnd, 0, 128, LWA_ALPHA);
    HRGN hRgn = CreateEllipticRgn(0, 0, 200, 100);
    SetWindowRgn(hOverlayWnd, hRgn, TRUE);

    // 6. Setup Tray Icon
    memset(&nid, 0, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hMainWnd;
    nid.uID = ID_TRAY_APP_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    _tcscpy_s(nid.szTip, _countof(nid.szTip), _T("Temp Monitor: Init..."));
    Shell_NotifyIcon(NIM_ADD, &nid);

    // 7. Start Timer
    SetTimer(hMainWnd, IDT_TIMER_UPDATE, 1000, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIcon(NIM_DELETE, &nid);
    monitor.cleanup();
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_TIMER:
        if (wParam == IDT_TIMER_UPDATE) {
            cpuTemp = monitor.getCPUTemp();
            gpuTemp = monitor.getGPUTemp();
            UpdateTrayTooltip();

            bool shouldShow = (cpuTemp >= thresholdTemp || gpuTemp >= thresholdTemp);
            bool shouldHide = (cpuTemp < (thresholdTemp - 5.0) && gpuTemp < (thresholdTemp - 5.0));

            if (shouldShow && !isOverlayVisible) ShowOverlay(true);
            else if (shouldHide && isOverlayVisible) ShowOverlay(false);

            if (isOverlayVisible) InvalidateRect(hOverlayWnd, NULL, FALSE);
        }
        break;

    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT curPoint;
            GetCursorPos(&curPoint);
            HMENU hMenu = CreatePopupMenu();
            
            // Auto-start checkbox
            AppendMenu(hMenu, MF_STRING | (isAutoStart ? MF_CHECKED : MF_UNCHECKED), ID_TRAY_AUTOSTART, _T("Run at Startup"));
            
            // Settings menu
            HMENU hSubMenu = CreateMenu();
            AppendMenu(hSubMenu, MF_STRING | (thresholdTemp == 60.0 ? MF_CHECKED : 0), 6001, _T("60 C"));
            AppendMenu(hSubMenu, MF_STRING | (thresholdTemp == 70.0 ? MF_CHECKED : 0), 6002, _T("70 C"));
            AppendMenu(hSubMenu, MF_STRING | (thresholdTemp == 80.0 ? MF_CHECKED : 0), 6003, _T("80 C"));
            AppendMenu(hSubMenu, MF_STRING | (thresholdTemp == 90.0 ? MF_CHECKED : 0), 6004, _T("90 C"));
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, _T("Set Threshold"));

            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, _T("Exit"));

            SetForegroundWindow(hWnd);
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_BOTTOMALIGN | TPM_LEFTALIGN, curPoint.x, curPoint.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);

            if (cmd == ID_TRAY_EXIT) {
                PostQuitMessage(0);
            } else if (cmd == ID_TRAY_AUTOSTART) {
                isAutoStart = !isAutoStart;
                SetStartup(isAutoStart);
            } else if (cmd >= 6001 && cmd <= 6004) {
                if (cmd == 6001) thresholdTemp = 60.0;
                if (cmd == 6002) thresholdTemp = 70.0;
                if (cmd == 6003) thresholdTemp = 80.0;
                if (cmd == 6004) thresholdTemp = 90.0;
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_NCHITTEST: {
        LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
        if (hit == HTCLIENT) return HTCAPTION;
        return hit;
    }
    case WM_EXITSIZEMOVE: {
        RECT rect;
        GetWindowRect(hWnd, &rect);
        lastOverlayPos.x = rect.left;
        lastOverlayPos.y = rect.top;
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
        SelectObject(hdc, hFont);

        RECT rect;
        GetClientRect(hWnd, &rect);
        
        std::wstringstream s1; s1 << L"CPU: " << (int)cpuTemp << L" C";
        std::wstringstream s2; s2 << L"GPU: " << (int)gpuTemp << L" C";
        
        std::wstring ws1 = s1.str();
        std::wstring ws2 = s2.str();
        
        RECT r1 = rect; r1.bottom -= 20;
        RECT r2 = rect; r2.top += 20;
        
        DrawTextW(hdc, ws1.c_str(), -1, &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        DrawTextW(hdc, ws2.c_str(), -1, &r2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        DeleteObject(hFont);
        EndPaint(hWnd, &ps);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void ShowOverlay(bool show) {
    isOverlayVisible = show;
    if (show) SetWindowPos(hOverlayWnd, HWND_TOPMOST, lastOverlayPos.x, lastOverlayPos.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    else ShowWindow(hOverlayWnd, SW_HIDE);
}

void UpdateTrayTooltip() {
    std::wstringstream ss;
    ss << L"CPU: " << (int)cpuTemp << L"C | GPU: " << (int)gpuTemp << L"C";
    std::wstring tip = ss.str();
    if (tip.length() >= 127) tip = tip.substr(0, 127);
    
    // Explicitly use 3 arguments for _tcscpy_s to match Windows API header requirements
    _tcscpy_s(nid.szTip, _countof(nid.szTip), tip.c_str());
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void SetStartup(bool enable) {
    HKEY hKey;
    const wchar_t* czRunPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* czAppName = L"TempMonitor";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, czRunPath, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        if (enable) {
            wchar_t szPath[MAX_PATH];
            GetModuleFileNameW(NULL, szPath, MAX_PATH);
            RegSetValueExW(hKey, czAppName, 0, REG_SZ, (LPBYTE)szPath, (lstrlenW(szPath) + 1) * sizeof(wchar_t));
        } else {
            RegDeleteValueW(hKey, czAppName);
        }
        RegCloseKey(hKey);
    }
}

bool CheckStartup() {
    HKEY hKey;
    const wchar_t* czRunPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* czAppName = L"TempMonitor";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, czRunPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        bool exists = (RegQueryValueExW(hKey, czAppName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS);
        RegCloseKey(hKey);
        return exists;
    }
    return false;
}
