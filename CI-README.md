# TCPING Cross-Platform CI/CD

This repository includes automated CI/CD pipeline for building TCPING across multiple platforms.

## 🚀 Automated Builds

The CI pipeline automatically builds TCPING for the following platforms when you push a git tag:

- **Linux x64** (Ubuntu latest)
- **macOS x64** (Intel-based Macs)
- **macOS ARM64** (Apple Silicon Macs)
- **Windows x64** (Windows latest)

## 📋 How to Trigger a Release

1. **Tag your commit:**

   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

2. **The CI will automatically:**
   - Build binaries for all platforms
   - Run basic tests
   - Create a GitHub release
   - Upload all binaries as release assets
   - Generate SHA256 checksums

## 🛠️ Manual Building

### Linux/macOS

```bash
cd tcping-src
chmod +x build.sh
./build.sh
```

### Windows

```cmd
cd tcping-src
build_windows.bat
```

## 📁 Release Assets

Each release will include:

- `tcping-linux-x64` - Linux binary
- `tcping-macos-x64` - macOS Intel binary
- `tcping-macos-arm64` - macOS Apple Silicon binary
- `tcping-windows-x64.exe` - Windows binary
- `SHA256SUMS.txt` - Checksums for verification
- `README.md` - Documentation
- `gpl-2.0.txt` - License file

## 🔐 Verification

Verify downloaded binaries using SHA256:

### Linux/macOS

```bash
sha256sum -c SHA256SUMS.txt
```

### Windows (PowerShell)

```powershell
Get-FileHash tcping-windows-x64.exe -Algorithm SHA256
```

## 🧪 Testing

The CI includes basic functionality tests:

- Help command execution
- Version information display
- Basic connectivity test

## 📝 CI Configuration

The CI pipeline is defined in `.github/workflows/release.yml` and includes:

### Build Matrix

- Cross-platform compilation
- Platform-specific toolchain setup
- Binary verification

### Release Creation

- Automatic release notes generation
- Asset upload
- Checksum generation

### Quality Assurance

- Build verification
- Basic functionality testing
- Asset integrity checks

## 🔧 Customization

### Adding New Platforms

To add support for additional platforms, update the matrix in `.github/workflows/release.yml`:

```yaml
matrix:
  include:
    - os: your-new-os
      target: your-target-name
      artifact_name: tcping
      asset_name: tcping-your-platform
```

### Modifying Build Options

Edit the build commands in the workflow file to customize:

- Compiler flags
- Optimization levels
- Feature flags
- Dependencies

## 🐛 Troubleshooting

### Build Failures

1. **Check build logs** in GitHub Actions tab
2. **Verify dependencies** are available on the target platform
3. **Test locally** using the platform-specific build scripts

### Missing Dependencies

The CI automatically installs required dependencies, but for local builds ensure you have:

- **Linux**: `build-essential`, `g++`
- **macOS**: Xcode command line tools
- **Windows**: Visual Studio 2019/2022 with C++ support

## 📊 Build Status

Check the status of builds at: `https://github.com/your-username/your-repo/actions`

## 🔄 Version Management

- Use semantic versioning (e.g., `v1.2.3`)
- Tag format: `v*` (e.g., `v1.0.0`, `v2.1.0-beta`)
- Each tag triggers a new release build

---

For more information about TCPING usage, see the main README.md file.
