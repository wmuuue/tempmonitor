#pragma once
#include <windows.h>
#include "TempMonitor.h"
#include "Config.h"

class FloatingWindow {
public:
    FloatingWindow(Config* config, TempMonitor* monitor);
    ~FloatingWindow();

    bool Create(HINSTANCE hInstance);
    void Show();
    void Hide();
    bool IsVisible() const { return visible; }
    
    void UpdateTemp(const TempData& data, int warningTemp, int dangerTemp);
    void SavePosition();

private:
    HWND hwnd;
    Config* config;
    TempMonitor* monitor;
    bool visible;
    bool dragging;
    POINT dragOffset;
    
    TempData currentData;
    int currentWarningTemp;
    int currentDangerTemp;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnPaint();
    void OnMouseDown(int x, int y);
    void OnMouseMove(int x, int y);
    void OnMouseUp();
    
    Gdiplus::Color GetColorForLevel(TempLevel level);
};
