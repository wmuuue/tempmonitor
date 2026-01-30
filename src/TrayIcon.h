#pragma once
#include <windows.h>
#include <shellapi.h>
#include "TempMonitor.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_SETTINGS 1001
#define ID_TRAY_EXIT 1002

class TrayIcon {
public:
    TrayIcon(HWND hwnd, TempMonitor* monitor);
    ~TrayIcon();

    bool Create(HINSTANCE hInstance);
    void Remove();
    void UpdateTooltip(const std::wstring& text);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND hwnd;
    TempMonitor* monitor;
    NOTIFYICONDATAW nid;
    HICON hIcon;

    void ShowContextMenu();
};
