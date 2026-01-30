#include "TrayIcon.h"
#include "resource.h"

TrayIcon::TrayIcon(HWND hwnd, TempMonitor* mon)
    : hwnd(hwnd), monitor(mon), hIcon(nullptr) {
    ZeroMemory(&nid, sizeof(nid));
}

TrayIcon::~TrayIcon() {
    Remove();
}

bool TrayIcon::Create(HINSTANCE hInstance) {
    hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    if (!hIcon) {
        hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }

    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hIcon;
    wcscpy_s(nid.szTip, L"Temperature Monitor");

    return Shell_NotifyIconW(NIM_ADD, &nid);
}

void TrayIcon::Remove() {
    if (nid.hWnd) {
        Shell_NotifyIconW(NIM_DELETE, &nid);
        nid.hWnd = NULL;
    }
    if (hIcon) {
        DestroyIcon(hIcon);
        hIcon = nullptr;
    }
}

void TrayIcon::UpdateTooltip(const std::wstring& text) {
    if (text.length() < 128) {
        wcscpy_s(nid.szTip, text.c_str());
        Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
}

void TrayIcon::ShowContextMenu() {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_SETTINGS, L"Settings");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

LRESULT TrayIcon::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_TRAYICON) {
        if (lParam == WM_RBUTTONUP) {
            ShowContextMenu();
            return 0;
        }
    }
    return 0;
}
