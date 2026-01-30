#include "Config.h"
#include <shlobj.h>
#include <sstream>

Config::Config() 
    : warningTemp(70), dangerTemp(85), windowX(-1), windowY(-1), autoStart(false) {
    
    // Get AppData path
    WCHAR appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        configPath = std::wstring(appDataPath) + L"\\TempMonitor";
        CreateDirectoryW(configPath.c_str(), NULL);
        configPath += L"\\config.ini";
    }
}

Config::~Config() {
}

bool Config::Load() {
    if (configPath.empty()) return false;

    warningTemp = GetPrivateProfileIntW(L"Thresholds", L"Warning", 70, configPath.c_str());
    dangerTemp = GetPrivateProfileIntW(L"Thresholds", L"Danger", 85, configPath.c_str());
    windowX = GetPrivateProfileIntW(L"Window", L"X", -1, configPath.c_str());
    windowY = GetPrivateProfileIntW(L"Window", L"Y", -1, configPath.c_str());
    autoStart = GetPrivateProfileIntW(L"General", L"AutoStart", 0, configPath.c_str()) != 0;

    return true;
}

bool Config::Save() {
    if (configPath.empty()) return false;

    WCHAR buffer[32];
    
    _itow_s(warningTemp, buffer, 10);
    WritePrivateProfileStringW(L"Thresholds", L"Warning", buffer, configPath.c_str());
    
    _itow_s(dangerTemp, buffer, 10);
    WritePrivateProfileStringW(L"Thresholds", L"Danger", buffer, configPath.c_str());
    
    _itow_s(windowX, buffer, 10);
    WritePrivateProfileStringW(L"Window", L"X", buffer, configPath.c_str());
    
    _itow_s(windowY, buffer, 10);
    WritePrivateProfileStringW(L"Window", L"Y", buffer, configPath.c_str());
    
    WritePrivateProfileStringW(L"General", L"AutoStart", autoStart ? L"1" : L"0", configPath.c_str());

    return true;
}

void Config::SetAutoStart(bool enable) {
    autoStart = enable;
    SetAutoStartRegistry(enable);
}

bool Config::SetAutoStartRegistry(bool enable) {
    HKEY hKey;
    const wchar_t* keyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* appName = L"TempMonitor";

    if (RegOpenKeyExW(HKEY_CURRENT_USER, keyPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    bool success = false;
    if (enable) {
        WCHAR exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        
        if (RegSetValueExW(hKey, appName, 0, REG_SZ, 
            (BYTE*)exePath, (wcslen(exePath) + 1) * sizeof(wchar_t)) == ERROR_SUCCESS) {
            success = true;
        }
    } else {
        if (RegDeleteValueW(hKey, appName) == ERROR_SUCCESS || 
            GetLastError() == ERROR_FILE_NOT_FOUND) {
            success = true;
        }
    }

    RegCloseKey(hKey);
    return success;
}

void Config::CreateDefaultConfig() {
    Save();
}
