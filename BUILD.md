# Building Temperature Monitor

This guide will help you build the Temperature Monitor application from source.

## Prerequisites

Before building, you need to install the following tools:

### 1. Visual Studio 2022

Download and install Visual Studio 2022 Community Edition (free):
- URL: https://visualstudio.microsoft.com/downloads/
- During installation, select **"Desktop development with C++"** workload
- This includes the MSVC compiler and Windows SDK

### 2. CMake

Download and install CMake:
- URL: https://cmake.org/download/
- Download the Windows x64 Installer
- During installation, select **"Add CMake to the system PATH"**

### 3. Icon File

Convert the PNG icon to ICO format:
- See `resources/ICON_README.md` for instructions
- Quick option: Use https://convertio.co/png-ico/

## Build Steps

### Option 1: Command Line Build

```powershell
# Navigate to project directory
cd d:\anti

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build Release version
cmake --build . --config Release

# The executable will be at: build\bin\Release\TempMonitor.exe
```

### Option 2: Visual Studio IDE

```powershell
# Generate Visual Studio solution
cd d:\anti
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64

# Open the generated solution
start TempMonitor.sln
```

Then in Visual Studio:
1. Select **Release** configuration
2. Build > Build Solution (Ctrl+Shift+B)
3. The executable will be in `build\bin\Release\TempMonitor.exe`

## Troubleshooting

### CMake not found
- Make sure CMake is installed and added to PATH
- Restart your terminal/PowerShell after installation
- Verify with: `cmake --version`

### MSVC not found
- Install Visual Studio 2022 with C++ development tools
- Make sure "Desktop development with C++" is selected

### Icon file missing
- See `resources/ICON_README.md`
- Convert `icon.png` to `icon.ico`
- Place it in the `resources` folder

### NVML not found (Runtime)
- NVML (nvml.dll) is included with NVIDIA GPU drivers
- If you don't have an NVIDIA GPU, GPU monitoring will be disabled
- CPU monitoring will still work

## Running the Application

After building:

```powershell
# Run from build directory
.\bin\Release\TempMonitor.exe

# Or copy to a permanent location
Copy-Item .\bin\Release\TempMonitor.exe C:\Tools\TempMonitor.exe
C:\Tools\TempMonitor.exe
```

The application will:
1. Minimize to system tray
2. Start monitoring temperatures
3. Show a floating window when temperature exceeds the threshold

## GitHub Actions

This project includes automated builds via GitHub Actions. When you push to GitHub:

1. The workflow automatically builds the application
2. The compiled executable is available as an artifact
3. You can download it from the Actions tab

To create a release:
```bash
git tag v1.0.0
git push origin v1.0.0
```

This will trigger a release build and attach the executable to the release.
