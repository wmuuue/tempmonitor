# Temperature Monitor - 温度监控软件

一个轻量级的 Windows 温度监控应用程序，支持 CPU 和 GPU 温度实时监控，带有可自定义的温度阈值提醒。

## 主要功能

- **实时温度监控**：每 2 秒监控一次 CPU 和 GPU 温度
- **NVIDIA GPU 支持**：显示 NVIDIA 显卡的温度和风扇转速
- **多级警报**：
  - 警告级别（默认 70°C）- 黄色指示
  - 危险级别（默认 85°C）- 红色指示
- **悬浮窗**：半透明（50% 不透明度）粉色椭圆形窗口显示温度
  - 温度达到警告阈值时自动显示
  - 温度低于上次触发温度 5°C 时自动隐藏
  - 可拖动，记忆位置
- **系统托盘集成**：
  - 最小化到系统托盘
  - 鼠标悬停显示当前温度
  - 右键菜单访问设置和退出
- **开机自启动选项**：可选的 Windows 启动集成（默认关闭）
- **轻量级**：最小内存占用

## 系统要求

- Windows 10/11（64 位）
- NVIDIA GPU 及驱动程序（用于 GPU 监控）
- Visual C++ Redistributable 2022 或更高版本

## 快速开始

### 下载预编译版本

从 [Releases](../../releases) 页面下载最新的 `TempMonitor.exe`

### 从源代码构建

详细构建说明请参见 [BUILD.md](BUILD.md)

简要步骤：
```powershell
# 安装 Visual Studio 2022 和 CMake
# 然后：
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## 使用方法

1. 运行 `TempMonitor.exe`
2. 应用程序将最小化到系统托盘
3. 鼠标悬停在托盘图标上查看当前温度
4. 右键点击托盘图标访问：
   - **设置**：配置温度阈值和开机自启动
   - **退出**：关闭应用程序

### 设置选项

- **警告温度**：悬浮窗显示的温度阈值（默认：70°C）
- **危险温度**：危险警报的温度阈值（默认：85°C）
- **开机启动**：启用/禁用系统启动时自动运行

## 工作原理

- **CPU 温度**：通过 Windows Management Instrumentation (WMI) 获取
- **GPU 温度**：通过 NVIDIA Management Library (NVML) 获取
- **风扇转速**：通过 NVML 获取（显示为百分比）

悬浮窗在 CPU 或 GPU 温度达到警告阈值时自动显示，当温度低于上次最高温度 5°C 时自动隐藏。

## 项目结构

```
d:/anti/
├── src/                    # 源代码
│   ├── main.cpp           # 程序入口
│   ├── TempMonitor.*      # 温度监控核心
│   ├── FloatingWindow.*   # 悬浮窗实现
│   ├── TrayIcon.*         # 系统托盘图标
│   ├── SettingsDialog.*   # 设置对话框
│   └── Config.*           # 配置管理
├── resources/             # 资源文件
│   ├── app.rc            # Windows 资源
│   └── icon.ico          # 应用图标
├── CMakeLists.txt        # CMake 构建配置
├── .github/workflows/    # GitHub Actions
└── README.md             # 本文件
```

## GitHub Actions 自动构建

本项目包含 GitHub Actions 自动构建配置。每次推送到主分支时：

1. 自动构建应用程序
2. 编译后的可执行文件作为 artifact 可供下载
3. 可从 Actions 标签页下载

创建发布版本：
```bash
git tag v1.0.0
git push origin v1.0.0
```

## 技术细节

- **语言**：C++17
- **UI 框架**：Win32 API + GDI+
- **温度监控**：WMI (CPU) + NVML (GPU)
- **构建系统**：CMake
- **CI/CD**：GitHub Actions

## 注意事项

- NVML (nvml.dll) 在运行时动态加载，包含在 NVIDIA GPU 驱动程序中
- 如果未检测到 NVIDIA GPU，GPU 监控将被禁用，但 CPU 监控仍可正常工作
- 应用程序使用互斥锁确保只运行一个实例

## 许可证

本项目采用 MIT 许可证开源。

## 贡献

欢迎提交问题和拉取请求！

## 支持

如有问题或建议，请在 GitHub Issues 中提出。
