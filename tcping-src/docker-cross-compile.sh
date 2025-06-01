#!/bin/bash
# 使用 Docker 在 macOS 上交叉编译 Linux 版本

echo "🐳 使用 Docker 在 macOS 上编译 Linux 版本"
echo

# 检查 Docker 是否可用
if ! command -v docker &> /dev/null; then
    echo "❌ Docker 未安装"
    echo "请安装 Docker: brew install --cask docker"
    exit 1
fi

# 检查 Docker 是否运行
if ! docker info &> /dev/null; then
    echo "❌ Docker 未运行，请启动 Docker Desktop"
    exit 1
fi

OUTPUT_NAME="tcping-linux-x64"

echo "🔨 使用 Docker 编译..."

# 使用 Ubuntu 容器进行编译
docker run --rm -v "$(pwd)":/work -w /work ubuntu:22.04 bash -c "
    echo '📦 更新包管理器...'
    apt-get update -qq
    
    echo '🔧 安装编译工具...'
    apt-get install -y -qq build-essential g++
    
    echo '🚀 开始编译...'
    g++ -O2 -std=c++11 -static-libgcc -static-libstdc++ \\
        -I. \\
        -D 'OS_LINUX' \\
        -D '_POSIX_C_SOURCE=199309L' \\
        main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp \\
        -o $OUTPUT_NAME
    
    echo '✅ 编译完成'
    ls -la $OUTPUT_NAME
    file $OUTPUT_NAME
"

if [[ $? -eq 0 && -f "$OUTPUT_NAME" ]]; then
    echo
    echo "🎉 Docker 交叉编译成功！"
    echo "📁 输出文件: $OUTPUT_NAME"
    ls -la $OUTPUT_NAME
    file $OUTPUT_NAME
    
    # 验证是否为 Linux 二进制文件
    if file $OUTPUT_NAME | grep -q "ELF.*x86-64"; then
        echo "✅ 确认生成了 Linux x86-64 二进制文件"
    else
        echo "⚠️ 生成的文件可能不是 Linux 二进制文件"
    fi
else
    echo "❌ Docker 编译失败"
    exit 1
fi
