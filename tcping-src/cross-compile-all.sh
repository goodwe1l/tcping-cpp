#!/bin/bash
# 使用 Zig 在 macOS 上交叉编译多个平台版本

echo "🚀 使用 Zig 进行跨平台编译"
echo

# 检查 Zig 是否可用
if ! command -v zig &> /dev/null; then
    echo "❌ Zig 编译器未找到"
    echo "请安装 Zig: brew install zig"
    exit 1
fi

echo "✅ 发现 Zig $(zig version)"
echo

# 创建输出目录
mkdir -p cross-build
cd cross-build

# 定义编译函数
compile_for_target() {
    local target=$1
    local output_name=$2
    local extra_flags=$3
    
    echo "🔨 编译 $target..."
    
    zig c++ \
        -target $target \
        -O2 \
        -std=c++11 \
        -I.. \
        $extra_flags \
        ../main.cpp ../tcping.cpp ../base64.cpp ../tee.cpp ../ws-util.cpp \
        -o $output_name
    
    if [[ $? -eq 0 ]]; then
        echo "✅ $target 编译成功: $output_name"
        ls -la $output_name
        file $output_name
        echo
    else
        echo "❌ $target 编译失败"
        echo
    fi
}

# 编译 Linux x86_64
compile_for_target "x86_64-linux-gnu" "tcping-linux-x64"

# 编译 Windows x86_64
compile_for_target "x86_64-windows-gnu" "tcping-windows-x64.exe" "-lws2_32"

# 编译 macOS x86_64 (Intel)
compile_for_target "x86_64-macos" "tcping-macos-x64"

# 编译 macOS ARM64 (Apple Silicon)
compile_for_target "aarch64-macos" "tcping-macos-arm64"

echo "🎉 跨平台编译完成！"
echo
echo "📁 生成的文件："
ls -la tcping-*
echo
echo "🔍 文件类型："
for file in tcping-*; do
    if [[ -f "$file" ]]; then
        echo "  $file:"
        file "$file" | sed 's/^/    /'
    fi
done

echo
echo "💡 使用方法："
echo "  # Linux 版本"
echo "  ./tcping-linux-x64 google.com 80"
echo
echo "  # Windows 版本"
echo "  ./tcping-windows-x64.exe google.com 80"
echo
echo "  # macOS 版本"
echo "  ./tcping-macos-x64 google.com 80"
echo "  ./tcping-macos-arm64 google.com 80"
