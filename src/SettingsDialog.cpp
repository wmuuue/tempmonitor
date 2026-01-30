#include "SettingsDialog.h"
#include <commctrl.h>
#include <sstream>

SettingsDialog::SettingsDialog(Config* cfg)
    : config(cfg), tempWarning(70), tempDanger(85), autoStart(false) {
}

SettingsDialog::~SettingsDialog() {
}

bool SettingsDialog::Show(HWND parent, HINSTANCE hInstance) {
    tempWarning = config->GetWarningTemp();
    tempDanger = config->GetDangerTemp();
    autoStart = config->GetAutoStart();

    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_SETTINGS), 
        parent, DialogProc, (LPARAM)this);

    return (result == IDOK);
}

INT_PTR CALLBACK SettingsDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    SettingsDialog* pThis = nullptr;

    if (uMsg == WM_INITDIALOG) {
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
        pThis = (SettingsDialog*)lParam;
        pThis->OnInitDialog(hwndDlg);
        return TRUE;
    } else {
        pThis = (SettingsDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    }

    if (pThis) {
        switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                pThis->OnOK(hwndDlg);
                EndDialog(hwndDlg, IDOK);
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                pThis->OnCancel(hwndDlg);
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
        }
    }

    return FALSE;
}

void SettingsDialog::OnInitDialog(HWND hwndDlg) {
    WCHAR buffer[32];
    
    _itow_s(tempWarning, buffer, 10);
    SetDlgItemTextW(hwndDlg, IDC_EDIT_WARNING, buffer);
    
    _itow_s(tempDanger, buffer, 10);
    SetDlgItemTextW(hwndDlg, IDC_EDIT_DANGER, buffer);
    
    CheckDlgButton(hwndDlg, IDC_CHECK_AUTOSTART, autoStart ? BST_CHECKED : BST_UNCHECKED);
}

void SettingsDialog::OnOK(HWND hwndDlg) {
    WCHAR buffer[32];
    
    GetDlgItemTextW(hwndDlg, IDC_EDIT_WARNING, buffer, 32);
    tempWarning = _wtoi(buffer);
    
    GetDlgItemTextW(hwndDlg, IDC_EDIT_DANGER, buffer, 32);
    tempDanger = _wtoi(buffer);
    
    autoStart = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_AUTOSTART) == BST_CHECKED);

    config->SetWarningTemp(tempWarning);
    config->SetDangerTemp(tempDanger);
    config->SetAutoStart(autoStart);
    config->Save();
}

void SettingsDialog::OnCancel(HWND hwndDlg) {
    // Do nothing
}
