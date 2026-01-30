#include "Config.h"
#include "TempMonitor.h"
#include "FloatingWindow.h"
#include "TrayIcon.h"
#include "SettingsDialog.h"
#include "resource.h"
#include <gdiplus.h>
#include <string>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Global variables
HINSTANCE g_hInstance = nullptr;
Config* g_config = nullptr;
TempMonitor* g_monitor = nullptr;
FloatingWindow* g_floatingWindow = nullptr;
TrayIcon* g_trayIcon = nullptr;
HWND g_hwndMain = nullptr;
bool g_windowShown = false;
float g_lastMaxTemp = 0.0f;

const int TIMER_UPDATE = 1;
const int UPDATE_INTERVAL = 2000; // 2 seconds

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnTimer();
void OnSettings();
void OnExit();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Ensure single instance
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"TempMonitorSingleInstanceMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return 0;
    }

    g_hInstance = hInstance;

    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Create main window (hidden)
    const wchar_t CLASS_NAME[] = L"TempMonitorMainWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);

    g_hwndMain = CreateWindowExW(
        0, CLASS_NAME, L"Temperature Monitor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hwndMain) {
        GdiplusShutdown(gdiplusToken);
        CloseHandle(hMutex);
        return 0;
    }

    // Initialize components
    g_config = new Config();
    g_config->Load();

    g_monitor = new TempMonitor();
    g_monitor->Initialize();

    g_floatingWindow = new FloatingWindow(g_config, g_monitor);
    g_floatingWindow->Create(hInstance);

    g_trayIcon = new TrayIcon(g_hwndMain, g_monitor);
    g_trayIcon->Create(hInstance);

    // Start update timer
    SetTimer(g_hwndMain, TIMER_UPDATE, UPDATE_INTERVAL, NULL);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    KillTimer(g_hwndMain, TIMER_UPDATE);
    
    delete g_trayIcon;
    delete g_floatingWindow;
    delete g_monitor;
    delete g_config;

    GdiplusShutdown(gdiplusToken);
    CloseHandle(hMutex);

    return 0;
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_TIMER:
        if (wParam == TIMER_UPDATE) {
            OnTimer();
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_SETTINGS) {
            OnSettings();
        } else if (LOWORD(wParam) == ID_TRAY_EXIT) {
            OnExit();
        }
        return 0;

    case WM_TRAYICON:
        return g_trayIcon->HandleMessage(uMsg, wParam, lParam);

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void OnTimer() {
    TempData data = g_monitor->GetCurrentTemp();
    
    if (!data.valid) return;

    int warningTemp = g_config->GetWarningTemp();
    int dangerTemp = g_config->GetDangerTemp();

    // Update tray tooltip
    std::wstring tooltipText = g_monitor->GetTempString();
    g_trayIcon->UpdateTooltip(tooltipText);

    // Check thresholds
    float maxTemp = (data.cpuTemp > data.gpuTemp) ? data.cpuTemp : data.gpuTemp;
    
    TempLevel cpuLevel = g_monitor->CheckThreshold(data.cpuTemp, warningTemp, dangerTemp);
    TempLevel gpuLevel = g_monitor->CheckThreshold(data.gpuTemp, warningTemp, dangerTemp);
    TempLevel maxLevel = (cpuLevel > gpuLevel) ? cpuLevel : gpuLevel;

    data.level = maxLevel;

    // Show/hide floating window based on threshold
    if (maxTemp >= warningTemp) {
        if (!g_windowShown) {
            g_floatingWindow->Show();
            g_windowShown = true;
        }
        g_floatingWindow->UpdateTemp(data, warningTemp, dangerTemp);
        g_lastMaxTemp = maxTemp;
    } else if (g_windowShown && maxTemp < (g_lastMaxTemp - 5.0f)) {
        g_floatingWindow->Hide();
        g_windowShown = false;
    } else if (g_windowShown) {
        g_floatingWindow->UpdateTemp(data, warningTemp, dangerTemp);
    }
}

void OnSettings() {
    SettingsDialog dialog(g_config);
    dialog.Show(g_hwndMain, g_hInstance);
}

void OnExit() {
    if (g_floatingWindow && g_floatingWindow->IsVisible()) {
        g_floatingWindow->SavePosition();
    }
    DestroyWindow(g_hwndMain);
}
