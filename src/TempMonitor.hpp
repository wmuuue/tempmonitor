#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <Wbemidl.h>
#include <comdef.h>

class TempMonitor {
public:
    TempMonitor();
    ~TempMonitor();

    bool initialize();
    double getCPUTemp();
    double getGPUTemp(); // Returns max of WMI or Nvidia temp
    void cleanup();

private:
    // WMI resources
    IWbemLocator* pLoc = NULL;
    IWbemServices* pSvc = NULL;
    bool wmiInitialized = false;
    
    // Nvidia resources
    HMODULE hNvml = NULL;
    void* nvDevice = NULL; // nvmlDevice_t
    bool nvmlInitialized = false;

    // Helpers
    double queryWMI(const std::wstring& query, const std::wstring& property);
    bool initNVML();
    double getNvidiaTemp();
};
