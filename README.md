# Temperature Monitor

A lightweight Windows application for monitoring CPU and GPU temperatures with customizable alerts.

## Features

- **Real-time Temperature Monitoring**: Monitors CPU and GPU temperatures every 2 seconds
- **NVIDIA GPU Support**: Displays GPU temperature and fan speed for NVIDIA graphics cards
- **Multi-level Alerts**: 
  - Warning level (default 70°C) - Yellow indicator
  - Danger level (default 85°C) - Red indicator
- **Floating Window**: Semi-transparent (50% opacity) pink oval window displaying temperatures
  - Appears when temperature reaches warning threshold
  - Auto-hides when temperature drops 5°C below last trigger
  - Draggable with position memory
- **System Tray Integration**: 
  - Minimizes to system tray
  - Hover tooltip shows current temperatures
  - Right-click menu for settings and exit
- **Auto-start Option**: Optional Windows startup integration (disabled by default)
- **Lightweight**: Minimal memory footprint

## Requirements

- Windows 10/11 (64-bit)
- NVIDIA GPU with drivers installed (for GPU monitoring)
- Visual C++ Redistributable 2022 or later

## Building from Source

### Prerequisites

- Visual Studio 2022 with C++ development tools
- CMake 3.15 or later
- Windows SDK

### Build Steps

```powershell
# Clone the repository
git clone <repository-url>
cd anti

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release

# Executable will be in: build/bin/Release/TempMonitor.exe
```

## Usage

1. Run `TempMonitor.exe`
2. The application will minimize to the system tray
3. Hover over the tray icon to see current temperatures
4. Right-click the tray icon to access:
   - **Settings**: Configure temperature thresholds and auto-start
   - **Exit**: Close the application

### Settings

- **Warning Temperature**: Temperature (°C) at which the floating window appears (default: 70°C)
- **Danger Temperature**: Temperature (°C) for critical alerts (default: 85°C)
- **Start with Windows**: Enable/disable auto-start on system boot

## How It Works

- **CPU Temperature**: Retrieved via Windows Management Instrumentation (WMI)
- **GPU Temperature**: Retrieved via NVIDIA Management Library (NVML)
- **Fan Speed**: Retrieved via NVML (displayed as percentage)

The floating window automatically appears when either CPU or GPU temperature reaches the warning threshold and disappears when temperatures drop 5°C below the last maximum temperature.

## GitHub Actions

This project includes automated builds via GitHub Actions. Every push to the main branch triggers a build, and the compiled executable is available as an artifact.

To create a release:
```bash
git tag v1.0.0
git push origin v1.0.0
```

## License

This project is open source and available under the MIT License.

## Notes

- NVML (nvml.dll) is loaded dynamically at runtime and is included with NVIDIA GPU drivers
- If no NVIDIA GPU is detected, GPU monitoring will be disabled but CPU monitoring will continue to work
- The application uses a mutex to ensure only one instance runs at a time
