# Quick Start Guide - 快速开始指南

## 🚀 快速开始（3 步）

### 步骤 1: 安装构建工具

1. **Visual Studio 2022**（免费）
   - 下载：https://visualstudio.microsoft.com/downloads/
   - 安装时选择 **"Desktop development with C++"**

2. **CMake**
   - 下载：https://cmake.org/download/
   - 安装时勾选 **"Add CMake to PATH"**

### 步骤 2: 准备图标文件

```powershell
# 方法 1: 在线转换（最简单）
# 1. 访问 https://convertio.co/png-ico/
# 2. 上传 d:\anti\resources\icon.png
# 3. 下载转换后的 icon.ico
# 4. 放到 d:\anti\resources\ 目录

# 方法 2: 使用 ImageMagick
# 安装后运行：
cd d:\anti\resources
magick icon.png -define icon:auto-resize=256,128,64,48,32,16 icon.ico
```

### 步骤 3: 构建并运行

```powershell
# 打开 PowerShell，进入项目目录
cd d:\anti

# 创建构建目录
mkdir build
cd build

# 配置项目
cmake .. -G "Visual Studio 17 2022" -A x64

# 编译（Release 版本）
cmake --build . --config Release

# 运行程序
.\bin\Release\TempMonitor.exe
```

---

## ✅ 验证安装

运行后，你应该看到：

1. ✅ 程序没有显示窗口（正常，它在托盘运行）
2. ✅ 系统托盘出现温度监控图标
3. ✅ 鼠标悬停在图标上显示当前温度
4. ✅ 右键图标可以看到"设置"和"退出"菜单

---

## 🎯 首次使用

### 配置温度阈值

1. 右键托盘图标 → **设置**
2. 设置警告温度（建议 70-75°C）
3. 设置危险温度（建议 85-90°C）
4. 可选：勾选"开机启动"
5. 点击"确定"保存

### 测试悬浮窗

运行一些高负载程序（如游戏、视频渲染）让温度升高，当温度达到警告值时：

- ✅ 自动显示粉色椭圆形悬浮窗
- ✅ 显示 CPU 和 GPU 实时温度
- ✅ 如果有风扇，显示风扇转速
- ✅ 可以拖动悬浮窗到任意位置
- ✅ 温度降低后自动隐藏

---

## 🔧 常见问题

### Q: CMake 提示找不到？
**A:** 重启 PowerShell 或重新安装 CMake 并确保勾选了"Add to PATH"

### Q: 编译失败，提示找不到 MSVC？
**A:** 确保安装了 Visual Studio 2022 的 C++ 开发工具

### Q: icon.ico 找不到？
**A:** 参见步骤 2，需要将 PNG 转换为 ICO 格式

### Q: 运行后看不到 GPU 温度？
**A:** 
- 确保你有 NVIDIA 显卡
- 确保安装了最新的 NVIDIA 驱动
- CPU 温度仍然可以正常显示

### Q: 悬浮窗不显示？
**A:** 
- 检查当前温度是否达到警告阈值
- 右键托盘图标 → 设置，查看阈值设置
- 尝试降低警告温度值进行测试

---

## 📦 GitHub Actions 自动构建

如果你不想本地构建，可以使用 GitHub Actions：

1. 将项目推送到 GitHub
2. 转到 **Actions** 标签页
3. 等待自动构建完成
4. 下载编译好的 `TempMonitor.exe`

创建发布版本：
```bash
git tag v1.0.0
git push origin v1.0.0
```

---

## 📚 更多信息

- 详细构建指南：[BUILD.md](file:///d:/anti/BUILD.md)
- 完整功能说明：[README_CN.md](file:///d:/anti/README_CN.md)
- 项目演练：查看 walkthrough.md

---

## 🎉 完成！

现在你已经成功构建并运行了温度监控软件！

**提示**：
- 托盘图标会实时更新温度信息
- 悬浮窗位置会自动保存
- 所有设置都保存在 `%APPDATA%\TempMonitor\config.ini`
