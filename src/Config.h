#pragma once
#include <windows.h>
#include <string>

class Config {
public:
    Config();
    ~Config();

    // Load configuration from file
    bool Load();
    
    // Save configuration to file
    bool Save();

    // Temperature thresholds
    int GetWarningTemp() const { return warningTemp; }
    void SetWarningTemp(int temp) { warningTemp = temp; }
    
    int GetDangerTemp() const { return dangerTemp; }
    void SetDangerTemp(int temp) { dangerTemp = temp; }

    // Window position
    int GetWindowX() const { return windowX; }
    void SetWindowX(int x) { windowX = x; }
    
    int GetWindowY() const { return windowY; }
    void SetWindowY(int y) { windowY = y; }

    // Auto-start
    bool GetAutoStart() const { return autoStart; }
    void SetAutoStart(bool enable);

    // Config file path
    std::wstring GetConfigPath() const { return configPath; }

private:
    std::wstring configPath;
    int warningTemp;
    int dangerTemp;
    int windowX;
    int windowY;
    bool autoStart;

    void CreateDefaultConfig();
    bool SetAutoStartRegistry(bool enable);
};
