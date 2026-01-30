#include "TempMonitor.hpp"
#include <iostream>

#pragma comment(lib, "wbemuuid.lib")

// NVML Definitions for dynamic loading
typedef int nvmlReturn_t;
#define NVML_SUCCESS 0
#define NVML_TEMPERATURE_GPU 0

typedef nvmlReturn_t(*nvmlInit_t)();
typedef nvmlReturn_t(*nvmlShutdown_t)();
typedef nvmlReturn_t(*nvmlDeviceGetHandleByIndex_t)(unsigned int, void*); // void* is nvmlDevice_t
typedef nvmlReturn_t(*nvmlDeviceGetTemperature_t)(void*, int, unsigned int*);

// Function pointers
nvmlInit_t pNvmlInit = nullptr;
nvmlShutdown_t pNvmlShutdown = nullptr;
nvmlDeviceGetHandleByIndex_t pNvmlDeviceGetHandleByIndex = nullptr;
nvmlDeviceGetTemperature_t pNvmlDeviceGetTemperature = nullptr;

TempMonitor::TempMonitor() {}

TempMonitor::~TempMonitor() {
    cleanup();
}

bool TempMonitor::initialize() {
    // 1. Initialize WMI
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) { 
        // It might be already initialized, which is fine
    }

    hres = CoInitializeSecurity(
        NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL
    );

    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (SUCCEEDED(hres)) {
        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\WMI"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
        if (FAILED(hres)) {
            hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
        }

        if (SUCCEEDED(hres)) {
            CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
                RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
            wmiInitialized = true;
        }
    }

    // 2. Initialize NVML (Dynamic loading)
    initNVML();

    return wmiInitialized || nvmlInitialized;
}

bool TempMonitor::initNVML() {
    hNvml = LoadLibraryA("nvml.dll"); // Try standard path
    if (!hNvml) {
        hNvml = LoadLibraryA("C:\\Program Files\\NVIDIA Corporation\\NVSMI\\nvml.dll");
    }

    if (hNvml) {
        pNvmlInit = (nvmlInit_t)GetProcAddress(hNvml, "nvmlInit");
        pNvmlShutdown = (nvmlShutdown_t)GetProcAddress(hNvml, "nvmlShutdown");
        pNvmlDeviceGetHandleByIndex = (nvmlDeviceGetHandleByIndex_t)GetProcAddress(hNvml, "nvmlDeviceGetHandleByIndex");
        pNvmlDeviceGetTemperature = (nvmlDeviceGetTemperature_t)GetProcAddress(hNvml, "nvmlDeviceGetTemperature");

        if (pNvmlInit && pNvmlShutdown && pNvmlDeviceGetHandleByIndex && pNvmlDeviceGetTemperature) {
            if (pNvmlInit() == NVML_SUCCESS) {
                // Get the first device (Index 0)
                if (pNvmlDeviceGetHandleByIndex(0, &nvDevice) == NVML_SUCCESS) {
                    nvmlInitialized = true;
                    return true;
                }
            }
        }
    }
    return false;
}

double TempMonitor::getCPUTemp() {
    if (!wmiInitialized) return 0.0;
    
    // MSAcpi_ThermalZoneTemperature is standard for many laptops
    double temp = queryWMI(L"SELECT CurrentTemperature FROM MSAcpi_ThermalZoneTemperature", L"CurrentTemperature");
    if (temp > 0) return (temp - 2732) / 10.0;

    return 0.0;
}

double TempMonitor::getGPUTemp() {
    double maxTemp = 0.0;

    // 1. Try Nvidia NVML first (usually more accurate for dGPU)
    if (nvmlInitialized) {
        double nvTemp = getNvidiaTemp();
        if (nvTemp > maxTemp) maxTemp = nvTemp;
    }

    // 2. Try WMI (CIM probe) - good for iGPU or if NVML fails
    if (wmiInitialized) {
        double wmiTemp = queryWMI(L"SELECT CurrentReading FROM Win32_TemperatureProbe", L"CurrentReading");
        if (wmiTemp > maxTemp) maxTemp = wmiTemp;
    }

    return maxTemp;
}

double TempMonitor::getNvidiaTemp() {
    if (!nvmlInitialized || !nvDevice) return 0.0;
    
    unsigned int temp = 0;
    if (pNvmlDeviceGetTemperature(nvDevice, NVML_TEMPERATURE_GPU, &temp) == NVML_SUCCESS) {
        return (double)temp;
    }
    return 0.0;
}

double TempMonitor::queryWMI(const std::wstring& query, const std::wstring& property) {
    if (!wmiInitialized) return 0.0;

    IEnumWbemClassObject* pEnumerator = NULL;
    HRESULT hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );

    if (FAILED(hres)) return 0.0;

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    double result = 0.0;

    if (pEnumerator) {
        // Only get the first result
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 != uReturn) {
            VARIANT vtProp;
            hr = pclsObj->Get(property.c_str(), 0, &vtProp, 0, 0);
            if (!FAILED(hr)) {
                if (vtProp.vt == VT_I4 || vtProp.vt == VT_UI4) {
                    result = (double)vtProp.ulVal;
                }
                VariantClear(&vtProp);
            }
            pclsObj->Release();
        }
        pEnumerator->Release();
    }

    return result;
}

void TempMonitor::cleanup() {
    if (nvmlInitialized && pNvmlShutdown) {
        pNvmlShutdown();
        nvmlInitialized = false;
    }
    if (hNvml) {
        FreeLibrary(hNvml);
        hNvml = NULL;
    }

    if (pSvc) { pSvc->Release(); pSvc = NULL; }
    if (pLoc) { pLoc->Release(); pLoc = NULL; }
    if (wmiInitialized) { CoUninitialize(); wmiInitialized = false; }
}
