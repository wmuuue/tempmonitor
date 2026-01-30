#include "TempMonitor.h"
#include <comdef.h>
#include <Wbemidl.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "wbemuuid.lib")

// NVML function pointers
typedef int (*nvmlInit_t)();
typedef int (*nvmlShutdown_t)();
typedef int (*nvmlDeviceGetHandleByIndex_t)(unsigned int, void**);
typedef int (*nvmlDeviceGetTemperature_t)(void*, int, unsigned int*);
typedef int (*nvmlDeviceGetFanSpeed_t)(void*, unsigned int*);

TempMonitor::TempMonitor() 
    : nvmlHandle(nullptr), gpuDevice(nullptr), nvmlInitialized(false) {
}

TempMonitor::~TempMonitor() {
    Shutdown();
}

bool TempMonitor::Initialize() {
    CoInitializeEx(0, COINIT_MULTITHREADED);
    InitNVML();
    return true;
}

void TempMonitor::Shutdown() {
    ShutdownNVML();
    CoUninitialize();
}

bool TempMonitor::InitNVML() {
    nvmlHandle = LoadLibraryA("nvml.dll");
    if (!nvmlHandle) {
        return false;
    }

    auto nvmlInit = (nvmlInit_t)GetProcAddress((HMODULE)nvmlHandle, "nvmlInit_v2");
    if (!nvmlInit) {
        nvmlInit = (nvmlInit_t)GetProcAddress((HMODULE)nvmlHandle, "nvmlInit");
    }
    
    if (!nvmlInit || nvmlInit() != 0) {
        FreeLibrary((HMODULE)nvmlHandle);
        nvmlHandle = nullptr;
        return false;
    }

    auto nvmlDeviceGetHandleByIndex = (nvmlDeviceGetHandleByIndex_t)
        GetProcAddress((HMODULE)nvmlHandle, "nvmlDeviceGetHandleByIndex_v2");
    if (!nvmlDeviceGetHandleByIndex) {
        nvmlDeviceGetHandleByIndex = (nvmlDeviceGetHandleByIndex_t)
            GetProcAddress((HMODULE)nvmlHandle, "nvmlDeviceGetHandleByIndex");
    }

    if (nvmlDeviceGetHandleByIndex && nvmlDeviceGetHandleByIndex(0, &gpuDevice) == 0) {
        nvmlInitialized = true;
        return true;
    }

    return false;
}

void TempMonitor::ShutdownNVML() {
    if (nvmlHandle) {
        auto nvmlShutdown = (nvmlShutdown_t)GetProcAddress((HMODULE)nvmlHandle, "nvmlShutdown");
        if (nvmlShutdown) {
            nvmlShutdown();
        }
        FreeLibrary((HMODULE)nvmlHandle);
        nvmlHandle = nullptr;
        nvmlInitialized = false;
    }
}

float TempMonitor::GetCPUTemp() {
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    float temp = 0.0f;

    HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) return 0.0f;

    // Try method 1: MSAcpi_ThermalZoneTemperature
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\WMI"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

    if (SUCCEEDED(hres)) {
        hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

        if (SUCCEEDED(hres)) {
            IEnumWbemClassObject* pEnumerator = nullptr;
            hres = pSvc->ExecQuery(
                bstr_t("WQL"),
                bstr_t("SELECT * FROM MSAcpi_ThermalZoneTemperature"),
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL, &pEnumerator);

            if (SUCCEEDED(hres)) {
                IWbemClassObject* pclsObj = nullptr;
                ULONG uReturn = 0;

                while (pEnumerator) {
                    HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                    if (0 == uReturn) break;

                    VARIANT vtProp;
                    hr = pclsObj->Get(L"CurrentTemperature", 0, &vtProp, 0, 0);
                    if (SUCCEEDED(hr)) {
                        temp = (vtProp.uintVal / 10.0f) - 273.15f;
                        VariantClear(&vtProp);
                        pclsObj->Release();
                        break;
                    }
                    pclsObj->Release();
                }
                pEnumerator->Release();
            }
        }
        pSvc->Release();
    }

    // If method 1 failed, try method 2: Win32_TemperatureProbe
    if (temp == 0.0f) {
        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
        
        if (SUCCEEDED(hres)) {
            hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

            if (SUCCEEDED(hres)) {
                IEnumWbemClassObject* pEnumerator = nullptr;
                hres = pSvc->ExecQuery(
                    bstr_t("WQL"),
                    bstr_t("SELECT * FROM Win32_TemperatureProbe"),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    NULL, &pEnumerator);

                if (SUCCEEDED(hres)) {
                    IWbemClassObject* pclsObj = nullptr;
                    ULONG uReturn = 0;

                    while (pEnumerator) {
                        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                        if (0 == uReturn) break;

                        VARIANT vtProp;
                        hr = pclsObj->Get(L"CurrentReading", 0, &vtProp, 0, 0);
                        if (SUCCEEDED(hr) && vtProp.uintVal > 0) {
                            temp = vtProp.uintVal / 10.0f;
                            VariantClear(&vtProp);
                            pclsObj->Release();
                            break;
                        }
                        VariantClear(&vtProp);
                        pclsObj->Release();
                    }
                    pEnumerator->Release();
                }
            }
            pSvc->Release();
        }
    }

    pLoc->Release();

    // Validate temperature range (reasonable CPU temp: 20-100°C)
    if (temp < 20.0f || temp > 100.0f) {
        temp = 0.0f;
    }

    return temp;
}

float TempMonitor::GetGPUTemp() {
    if (!nvmlInitialized || !gpuDevice || !nvmlHandle) {
        return 0.0f;
    }

    auto nvmlDeviceGetTemperature = (nvmlDeviceGetTemperature_t)
        GetProcAddress((HMODULE)nvmlHandle, "nvmlDeviceGetTemperature");
    
    if (!nvmlDeviceGetTemperature) {
        return 0.0f;
    }

    unsigned int temp = 0;
    if (nvmlDeviceGetTemperature(gpuDevice, 0, &temp) == 0) {
        return (float)temp;
    }

    return 0.0f;
}

int TempMonitor::GetFanSpeed() {
    if (!nvmlInitialized || !gpuDevice || !nvmlHandle) {
        return 0;
    }

    auto nvmlDeviceGetFanSpeed = (nvmlDeviceGetFanSpeed_t)
        GetProcAddress((HMODULE)nvmlHandle, "nvmlDeviceGetFanSpeed");
    
    if (!nvmlDeviceGetFanSpeed) {
        return 0;
    }

    unsigned int speed = 0;
    if (nvmlDeviceGetFanSpeed(gpuDevice, &speed) == 0) {
        return (int)speed;
    }

    return 0;
}

TempData TempMonitor::GetCurrentTemp() {
    TempData data;
    data.cpuTemp = GetCPUTemp();
    data.gpuTemp = GetGPUTemp();
    data.fanSpeed = GetFanSpeed();
    data.valid = (data.cpuTemp > 0 || data.gpuTemp > 0);
    data.level = TempLevel::Normal;
    
    return data;
}

TempLevel TempMonitor::CheckThreshold(float temp, int warningTemp, int dangerTemp) {
    if (temp >= dangerTemp) {
        return TempLevel::Danger;
    } else if (temp >= warningTemp) {
        return TempLevel::Warning;
    }
    return TempLevel::Normal;
}

std::wstring TempMonitor::GetTempString() {
    TempData data = GetCurrentTemp();
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << L"CPU: " << data.cpuTemp << L"°C | GPU: " << data.gpuTemp << L"°C";
    if (data.fanSpeed > 0) {
        ss << L" | Fan: " << data.fanSpeed << L"%";
    }
    return ss.str();
}
