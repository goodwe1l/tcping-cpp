# GitHub CI/CD 流水线使用指南

## 🎯 概述

TCPing 项目现在支持全自动的跨平台构建，使用 **Zig** 作为交叉编译器，可以在单个 Ubuntu 22.04 环境中构建所有目标平台的二进制文件。

## 🔧 现有 CI/CD 流水线

### 1. **持续集成** (`ci.yml`)
- **触发条件**: 推送到 main/master/develop 分支，或创建 PR
- **运行环境**: Ubuntu 22.04, macOS 12, Windows 2022
- **功能**: 编译测试，确保代码在各平台能正常构建

### 2. **简单发布** (`release-simple.yml`)
- **触发条件**: 推送 `v*` 标签
- **运行环境**: 多平台原生编译
- **功能**: 为每个平台构建原生二进制文件并发布

### 3. **Zig 交叉编译** (`cross-compile.yml`) ⭐ **新增**
- **触发条件**: 推送代码或标签
- **运行环境**: 仅 Ubuntu 22.04
- **功能**: 使用 Zig 交叉编译所有平台二进制文件

## 🚀 Zig 交叉编译优势

### ✅ 优点
- **速度快**: 只需要一个 Ubuntu runner
- **一致性**: 所有二进制文件在同一环境构建
- **成本低**: 减少 CI/CD 使用时间
- **可靠性**: Zig 提供优秀的交叉编译支持

### 📊 生成的文件
| 平台 | 文件名 | 格式 | 大小 |
|------|--------|------|------|
| Linux x64 | `tcping-linux-x64` | ELF 64-bit | ~5MB |
| macOS x64 | `tcping-macos-x64` | Mach-O x64 | ~600KB |
| macOS ARM64 | `tcping-macos-arm64` | Mach-O ARM64 | ~680KB |
| Windows x64 | `tcping-windows-x64.exe` | PE32+ | ~850KB |

## 🎮 使用方法

### 自动发布
1. 创建新标签:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

2. GitHub Actions 自动:
   - 交叉编译所有平台
   - 创建 GitHub Release
   - 上传所有二进制文件

### 手动构建
本地使用交叉编译脚本:
```bash
# 所有平台
./cross-compile-all.sh

# 单独平台
./cross-compile-linux.sh
./cross-compile-windows.sh

# 简化版本
./cross-compile-simple.sh
```

## 🔍 工作流选择建议

### 开发阶段
- 使用 `ci.yml` 进行日常 CI 测试
- 推送到开发分支触发自动构建验证

### 发布阶段
- **推荐**: 使用 `cross-compile.yml` - 快速、一致
- **备选**: 使用 `release-simple.yml` - 原生编译、更大兼容性

### 兼容性测试
- 使用 `ci.yml` 在真实的多平台环境中测试
- 确保代码在各个操作系统中正确运行

## 📝 配置更新记录

### ✅ 已完成
- [x] 修复 macOS/Linux 编译错误
- [x] 更新 Ubuntu 20.04 → 22.04
- [x] 实现 Zig 交叉编译
- [x] 创建多平台构建脚本
- [x] 修复 Windows 交叉编译问题

### 🔧 技术细节
- **编译器**: Zig 0.14.1
- **C++ 标准**: C++11
- **优化级别**: -O2
- **目标平台**: x86_64-linux-gnu, x86_64-windows-gnu, x86_64-macos, aarch64-macos

## 🚨 故障排除

### 常见问题
1. **Windows 链接错误**: 确保添加 `-lws2_32` 参数
2. **宏重定义警告**: 正常现象，不影响编译
3. **文件大小差异**: Linux 版本包含调试信息，较大

### 验证构建
```bash
# 检查文件类型
file tcping-*

# 测试运行
./tcping-macos-x64 google.com 80
```

## 🎯 下一步计划

- [ ] 添加自动化测试脚本
- [ ] 集成代码签名（macOS/Windows）
- [ ] 添加性能基准测试
- [ ] 支持更多架构（ARM Linux）
