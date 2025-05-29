#!/bin/bash
# 通用构建脚本，用于在Linux和macOS上编译tcping

echo "开始构建tcping跨平台版..."
echo

# 检测操作系统
OS=$(uname -s)
case "$OS" in
  Linux*)  
    echo "检测到Linux系统"
    ;;
  Darwin*) 
    echo "检测到macOS系统"
    ;;
  *)
    echo "未知操作系统: $OS"
    exit 1
    ;;
esac

# 清理旧文件
echo "清理旧文件..."
rm -f tcping *.o

# 编译
echo "编译中..."
make clean
make

# 检查是否编译成功
if [ -f "tcping" ]; then
    echo
    echo "构建成功!"
    echo "可以使用 ./tcping [选项] 主机地址 [端口] 运行程序"
    
    # 添加可执行权限
    chmod +x tcping
    
    # 显示程序大小
    ls -lh tcping
else
    echo
    echo "构建失败，请检查错误信息"
    exit 1
fi
