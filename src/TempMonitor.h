#pragma once
#include <windows.h>
#include <string>

enum class TempLevel {
    Normal,
    Warning,
    Danger
};

struct TempData {
    float cpuTemp;
    float gpuTemp;
    int fanSpeed;
    TempLevel level;
    bool valid;
};

class TempMonitor {
public:
    TempMonitor();
    ~TempMonitor();

    bool Initialize();
    void Shutdown();
    
    TempData GetCurrentTemp();
    TempLevel CheckThreshold(float temp, int warningTemp, int dangerTemp);
    
    std::wstring GetTempString();

private:
    void* nvmlHandle;
    void* gpuDevice;
    bool nvmlInitialized;

    float GetCPUTemp();
    float GetGPUTemp();
    int GetFanSpeed();
    
    bool InitNVML();
    void ShutdownNVML();
};
