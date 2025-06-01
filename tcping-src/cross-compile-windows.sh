#!/bin/bash
# 在 macOS 上交叉编译 Windows 版本的脚本

echo "🔨 开始在 macOS 上交叉编译 Windows 版本..."
echo

# 检查是否在 macOS 上运行
if [[ "$(uname)" != "Darwin" ]]; then
    echo "❌ 此脚本只能在 macOS 上运行"
    exit 1
fi

# 设置目标平台
TARGET_TRIPLE="x86_64-windows-gnu"
OUTPUT_NAME="tcping-windows-x64.exe"

# 检查是否安装了 Zig（推荐方法）
if command -v zig &> /dev/null; then
    echo "✅ 发现 Zig 编译器，使用 Zig 进行交叉编译..."
    
    # 使用 Zig 作为交叉编译器
    zig c++ \
        -target x86_64-windows-gnu \
        -O2 \
        -std=c++11 \
        -I. \
        main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp \
        -lws2_32 \
        -o $OUTPUT_NAME
    
    if [[ $? -eq 0 ]]; then
        echo "✅ 使用 Zig 交叉编译成功！"
        file $OUTPUT_NAME
        ls -la $OUTPUT_NAME
        echo
        echo "📦 生成的 Windows 可执行文件: $OUTPUT_NAME"
        echo "🎯 目标平台: Windows x86_64"
        echo "📁 文件位置: $(pwd)/$OUTPUT_NAME"
        exit 0
    else
        echo "❌ Zig 交叉编译失败，尝试其他方法..."
    fi
fi

# 方法 2: 尝试使用 MinGW-w64（如果安装了）
echo "🔧 尝试使用 MinGW-w64 进行交叉编译..."

# 检查可能的 MinGW-w64 编译器
MINGW_COMPILERS=(
    "x86_64-w64-mingw32-g++"
    "/usr/local/bin/x86_64-w64-mingw32-g++"
    "/opt/homebrew/bin/x86_64-w64-mingw32-g++"
    "/usr/local/Cellar/mingw-w64/*/toolchain-x86_64/bin/x86_64-w64-mingw32-g++"
)

MINGW_COMPILER=""
for compiler in "${MINGW_COMPILERS[@]}"; do
    if command -v "$compiler" &> /dev/null || [[ -f "$compiler" ]]; then
        MINGW_COMPILER="$compiler"
        break
    fi
done

if [[ -n "$MINGW_COMPILER" ]]; then
    echo "✅ 找到 MinGW-w64 编译器: $MINGW_COMPILER"
    
    $MINGW_COMPILER \
        -O2 \
        -std=c++11 \
        -I. \
        -D "OS_WIN" \
        -D "_WIN32_WINNT=0x0601" \
        -static-libgcc \
        -static-libstdc++ \
        main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp \
        -lws2_32 \
        -o $OUTPUT_NAME
    
    if [[ $? -eq 0 ]]; then
        echo "✅ 使用 MinGW-w64 交叉编译成功！"
        file $OUTPUT_NAME
        ls -la $OUTPUT_NAME
        echo
        echo "📦 生成的 Windows 可执行文件: $OUTPUT_NAME"
        echo "🎯 目标平台: Windows x86_64"
        echo "📁 文件位置: $(pwd)/$OUTPUT_NAME"
        exit 0
    fi
fi

# 方法 3: 使用 Docker + Wine 进行测试编译
if command -v docker &> /dev/null; then
    echo "🐳 尝试使用 Docker 进行交叉编译..."
    
    docker run --rm -v "$(pwd)":/work -w /work \
        dockcross/windows-static-x64 \
        bash -c "
            x86_64-w64-mingw32.static-g++ \
                -O2 \
                -std=c++11 \
                -I. \
                -D 'OS_WIN' \
                -D '_WIN32_WINNT=0x0601' \
                -static \
                main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp \
                -lws2_32 \
                -o $OUTPUT_NAME
        "
    
    if [[ $? -eq 0 ]]; then
        echo "✅ 使用 Docker 交叉编译成功！"
        file $OUTPUT_NAME
        ls -la $OUTPUT_NAME
        echo
        echo "📦 生成的 Windows 可执行文件: $OUTPUT_NAME"
        echo "🎯 目标平台: Windows x86_64"
        echo "📁 文件位置: $(pwd)/$OUTPUT_NAME"
        exit 0
    fi
fi

# 如果所有方法都失败
echo "❌ Windows 交叉编译失败！"
echo
echo "💡 建议的解决方案："
echo "1. 安装 Zig: brew install zig"
echo "2. 安装 MinGW-w64: brew install mingw-w64"
echo "3. 安装 Docker: brew install --cask docker"
echo "4. 使用 CI/CD 流水线自动构建"
echo
echo "🔧 安装说明："
echo "   brew install zig           # 推荐方法"
echo "   brew install mingw-w64     # 替代方法"
echo "   brew install --cask docker # Docker 方法"
echo
exit 1
