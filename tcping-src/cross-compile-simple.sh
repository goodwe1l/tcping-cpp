#!/bin/bash
# 简化版交叉编译脚本 - 在 macOS 上为 Linux 交叉编译

echo "🔨 在 macOS 上交叉编译 Linux 版本 (简化版)"
echo

# 检查当前系统
if [[ "$(uname)" != "Darwin" ]]; then
    echo "❌ 此脚本需要在 macOS 上运行"
    exit 1
fi

OUTPUT_NAME="tcping-linux-x64"

# 清理旧文件
echo "🧹 清理旧文件..."
rm -f *.o $OUTPUT_NAME

# 方法 1: 使用 osxcross（如果安装了）
if command -v x86_64-linux-gnu-g++ &> /dev/null; then
    echo "✅ 发现 osxcross 工具链"
    
    x86_64-linux-gnu-g++ \
        -O2 -std=c++11 \
        -static \
        -I. \
        -D "OS_LINUX" \
        -D "_POSIX_C_SOURCE=199309L" \
        main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp \
        -o $OUTPUT_NAME
    
    if [[ $? -eq 0 ]]; then
        echo "✅ osxcross 交叉编译成功！"
        file $OUTPUT_NAME
        ls -la $OUTPUT_NAME
        exit 0
    fi
fi

# 方法 2: 使用自定义的 musl 交叉编译器
if command -v musl-gcc &> /dev/null; then
    echo "✅ 发现 musl 编译器"
    
    musl-gcc \
        -O2 -std=c++11 \
        -static \
        -I. \
        -D "OS_LINUX" \
        -D "_POSIX_C_SOURCE=199309L" \
        main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp \
        -lstdc++ \
        -o $OUTPUT_NAME
    
    if [[ $? -eq 0 ]]; then
        echo "✅ musl 交叉编译成功！"
        file $OUTPUT_NAME
        ls -la $OUTPUT_NAME
        exit 0
    fi
fi

# 方法 3: 提示用户安装工具或使用其他方法
echo "❌ 未找到合适的交叉编译工具链"
echo
echo "💡 可用的解决方案："
echo
echo "1. 📦 安装 Docker 并使用容器编译:"
echo "   brew install --cask docker"
echo "   然后运行: ./docker-cross-compile.sh"
echo
echo "2. 🔧 安装 OSX Cross toolchain:"
echo "   brew tap SergioBenitez/osxct"
echo "   brew install x86_64-unknown-linux-gnu"
echo
echo "3. 🐧 在真实的 Linux 系统上编译"
echo
echo "4. 🚀 使用 GitHub Actions CI/CD:"
echo "   git tag v1.0.0 && git push origin v1.0.0"
echo
echo "5. ⚡ 等待 Zig 安装完成后重试"
echo
exit 1
