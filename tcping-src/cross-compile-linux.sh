#!/bin/bash
# 在 macOS 上交叉编译 Linux 版本的脚本

echo "🔨 开始在 macOS 上交叉编译 Linux 版本..."
echo

# 检查是否在 macOS 上运行
if [[ "$(uname)" != "Darwin" ]]; then
    echo "❌ 此脚本只能在 macOS 上运行"
    exit 1
fi

# 设置目标平台
TARGET_TRIPLE="x86_64-linux-gnu"
OUTPUT_NAME="tcping-linux-x64"

# 检查是否安装了 Zig（推荐方法）
if command -v zig &> /dev/null; then
    echo "✅ 发现 Zig 编译器，使用 Zig 进行交叉编译..."
    
    # 使用 Zig 作为交叉编译器
    zig c++ \
        -target x86_64-linux-gnu \
        -O2 \
        -std=c++11 \
        -I. \
        main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp \
        -o $OUTPUT_NAME
    
    if [[ $? -eq 0 ]]; then
        echo "✅ 使用 Zig 交叉编译成功！"
        file $OUTPUT_NAME
        ls -la $OUTPUT_NAME
        exit 0
    else
        echo "❌ Zig 交叉编译失败，尝试其他方法..."
    fi
fi

# 方法 2: 尝试使用 Clang 交叉编译（需要额外配置）
echo "🔧 尝试使用 Clang 进行交叉编译..."

# 检查是否有合适的 sysroot
POSSIBLE_SYSROOTS=(
    "/usr/local/x86_64-linux-gnu"
    "/opt/cross/x86_64-linux-gnu"
    "/usr/x86_64-linux-gnu"
)

SYSROOT=""
for path in "${POSSIBLE_SYSROOTS[@]}"; do
    if [[ -d "$path" ]]; then
        SYSROOT="$path"
        break
    fi
done

if [[ -n "$SYSROOT" ]]; then
    echo "✅ 找到 sysroot: $SYSROOT"
    
    clang++ \
        --target=x86_64-linux-gnu \
        --sysroot="$SYSROOT" \
        -O2 \
        -std=c++11 \
        -I. \
        -D "OS_LINUX" \
        -D "_POSIX_C_SOURCE=199309L" \
        main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp \
        -o $OUTPUT_NAME
    
    if [[ $? -eq 0 ]]; then
        echo "✅ 使用 Clang 交叉编译成功！"
        file $OUTPUT_NAME
        ls -la $OUTPUT_NAME
        exit 0
    fi
fi

# 方法 3: 使用 Docker（如果可用）
if command -v docker &> /dev/null; then
    echo "🐳 尝试使用 Docker 进行交叉编译..."
    
    docker run --rm -v "$(pwd)":/work -w /work \
        gcc:latest \
        bash -c "
            g++ -O2 -std=c++11 -I. \
                -D 'OS_LINUX' \
                -D '_POSIX_C_SOURCE=199309L' \
                main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp \
                -o $OUTPUT_NAME
        "
    
    if [[ $? -eq 0 ]]; then
        echo "✅ 使用 Docker 交叉编译成功！"
        file $OUTPUT_NAME
        ls -la $OUTPUT_NAME
        exit 0
    fi
fi

# 如果所有方法都失败
echo "❌ 交叉编译失败！"
echo
echo "💡 建议的解决方案："
echo "1. 安装 Zig: brew install zig"
echo "2. 安装 Docker: brew install --cask docker"
echo "3. 在 Linux 系统上直接编译"
echo "4. 使用 CI/CD 流水线自动构建"
echo
exit 1
