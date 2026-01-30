#pragma once
#include <windows.h>
#include "Config.h"

#define IDD_SETTINGS 101
#define IDC_EDIT_WARNING 1001
#define IDC_EDIT_DANGER 1002
#define IDC_CHECK_AUTOSTART 1003
#define IDOK 1
#define IDCANCEL 2

class SettingsDialog {
public:
    SettingsDialog(Config* config);
    ~SettingsDialog();

    bool Show(HWND parent, HINSTANCE hInstance);

private:
    Config* config;
    int tempWarning;
    int tempDanger;
    bool autoStart;

    static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnInitDialog(HWND hwndDlg);
    void OnOK(HWND hwndDlg);
    void OnCancel(HWND hwndDlg);
};
