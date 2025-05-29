### 1. GitHub Actions 工作流

- `.github/workflows/release.yml` - 完整的发布流水线（推荐）
- `.github/workflows/release-simple.yml` - 简化版发布流水线
- `.github/workflows/ci.yml` - 持续集成测试流水线

### 2. 构建脚本

- `tcping-src/build_windows.bat` - Windows 专用构建脚本

### 3. 文档

- `CI-README.md` - 详细的 CI/CD 使用说明

## 🎯 支持的平台

自动构建将生成以下平台的二进制文件：

| 平台    | 架构          | 文件名                   | 运行环境      |
| ------- | ------------- | ------------------------ | ------------- |
| Linux   | x64           | `tcping-linux-x64`       | Ubuntu 20.04+ |
| macOS   | x64 (Intel)   | `tcping-macos-x64`       | macOS 10.15+  |
| macOS   | ARM64 (M1/M2) | `tcping-macos-arm64`     | macOS 11.0+   |
| Windows | x64           | `tcping-windows-x64.exe` | Windows 10+   |

## 🚀 如何使用

### 步骤 1: 设置 GitHub 仓库

1. 将你的代码推送到 GitHub 仓库
2. 确保仓库包含所有必要的文件

### 步骤 2: 触发自动构建

有两种方式触发构建：

#### 方式 A: 发布版本（推荐）

```bash
# 创建并推送标签
git tag v1.0.0
git push origin v1.0.0
```

#### 方式 B: 推送代码（仅测试构建）

```bash
# 推送到主分支将触发 CI 测试
git push origin main
```

### 步骤 3: 检查构建状态

1. 访问你的 GitHub 仓库
2. 点击 "Actions" 标签页
3. 查看构建进度和日志

### 步骤 4: 下载发布文件

构建完成后：

1. 访问 "Releases" 页面
2. 下载对应平台的二进制文件
3. 使用 `SHA256SUMS.txt` 验证文件完整性

## ⚙️ 配置文件说明

### 主要工作流 (release.yml)

```yaml
# 触发条件：推送 v* 标签
on:
  push:
    tags:
      - "v*"

# 构建矩阵：4个平台并行构建
strategy:
  matrix:
    include:
      - os: ubuntu-latest # Linux x64
      - os: macos-13 # macOS Intel
      - os: macos-latest # macOS ARM64
      - os: windows-latest # Windows x64
```

### 构建步骤

1. **检出代码** - 获取源代码
2. **设置环境** - 安装编译工具链
3. **编译** - 使用平台特定的构建脚本
4. **测试** - 验证二进制文件
5. **打包** - 准备发布文件
6. **发布** - 创建 GitHub Release

## 🛠️ 自定义配置

### 修改支持的平台

编辑 `.github/workflows/release.yml` 中的 matrix 部分：

```yaml
matrix:
  include:
    # 添加新平台
    - os: ubuntu-20.04
      asset_name: tcping-linux-ubuntu20
    # 移除不需要的平台（注释掉即可）
    # - os: windows-latest
```

### 修改编译选项

编辑构建脚本：

- Linux/macOS: 修改 `tcping-src/build.sh`
- Windows: 修改 `tcping-src/build_windows.bat`

### 自定义发布说明

编辑 `.github/workflows/release.yml` 中的 `body` 部分。

## 🧪 本地测试

在推送标签前，可以本地测试构建：

```bash
# Linux/macOS
cd tcping-src
./build.sh

# Windows
cd tcping-src
build_windows.bat
```

## 📊 监控和调试

### 查看构建日志

1. GitHub → Actions → 选择工作流运行
2. 点击具体的 job 查看详细日志

### 常见问题排查

1. **构建失败**: 检查编译错误日志
2. **测试失败**: 检查二进制文件是否正确生成
3. **发布失败**: 检查 GitHub Token 权限

## 🔐 安全说明

- CI 使用 GitHub 自动生成的 `GITHUB_TOKEN`
- 无需额外配置 secrets
- 所有操作都在 GitHub 安全环境中执行

## 📈 版本管理建议

### 标签命名规范

```bash
v1.0.0      # 主要版本
v1.1.0      # 功能更新
v1.1.1      # 错误修复
v2.0.0-beta # 测试版本
```

### 自动语义化版本

可以考虑使用工具如 `semantic-release` 自动管理版本号。

## 🎉 完成！

现在你的 tcping 项目已经配置了完整的 CI/CD 流水线！

**下次发布新版本时，只需要：**

```bash
git tag v1.0.0
git push origin v1.0.0
```

**就会自动：**

- ✅ 构建 4 个平台的二进制文件
- ✅ 运行测试验证功能
- ✅ 创建 GitHub Release
- ✅ 上传所有构建产物
- ✅ 生成完整的发布说明

🚀 **开始使用你的自动化构建流水线吧！**
