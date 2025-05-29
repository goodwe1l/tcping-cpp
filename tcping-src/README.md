# tcping 跨平台版

tcping 是一个 TCP 端口探测工具，类似于常用的 ping 命令，但是它使用 TCP 协议而非 ICMP 协议。这使得 tcping 能够在一些只允许 TCP 通信但阻止 ICMP 的网络环境中工作。

这个版本是原始 tcping 工具的跨平台适配版，支持 Windows、Linux 和 macOS。

## 功能特性

- TCP 端口连接测试
- 连接响应时间测量（类似 ping 的延迟测量）
- 支持 HTTP 请求（GET/HEAD/POST）
- 支持代理服务器
- 支持 IPv4/IPv6
- 连续测试和结果统计
- 结果可以导出到日志文件
- 跨平台支持（Windows/Linux/macOS）

## 编译指南

### Windows

Windows 环境下有两种编译方式：

1. **使用 Visual Studio 命令行**:

   ```
   cd tcping-src
   make.bat
   ```

2. **使用 Make**:
   ```
   cd tcping-src
   nmake
   ```

### Linux

```bash
cd tcping-src
./build.sh
```

或者手动编译：

```bash
cd tcping-src
make clean
make
```

### macOS

```bash
cd tcping-src
./build.sh
```

或者手动编译：

```bash
cd tcping-src
make clean
make
```

## 使用方法

基本语法：

```
tcping [-选项] 主机地址 [端口]
```

完整语法：

```
tcping [-t] [-d] [-i 间隔] [-n 次数] [-w 毫秒] [-b n] [-r 次数] [-s] [-v] [-j]
       [-js 样本大小] [-4] [-6] [-c] [-g 次数] [-S 源地址] [--file] [--tee 文件名]
       [-h] [-u] [--post] [--head] [--proxy-port 端口] [--proxy-server 服务器]
       [--proxy-credentials 用户名:密码] [-f] 主机地址 [端口]
```

### 常用选项

- `-t` : 持续 ping 直到按下 Ctrl+C 停止
- `-n 5` : 例如，发送 5 次 ping
- `-i 5` : 例如，每 5 秒 ping 一次
- `-w 0.5` : 例如，等待响应 0.5 秒
- `-d` : 在每行包含日期和时间
- `-b 1` : 启用蜂鸣声（1 表示连接断开时蜂鸣，2 表示连接启动时蜂鸣，
  3 表示在状态变化时蜂鸣，4 表示总是蜂鸣）
- `-r 5` : 例如，每 5 次 ping 重新解析主机名
- `-s` : 在成功 ping 通后自动退出
- `-v` : 打印版本并退出
- `-j` : 包含抖动信息，使用默认滚动平均值
- `-4` : 优先使用 IPv4
- `-6` : 优先使用 IPv6

### HTTP 选项

- `-h` : HTTP 模式（使用不带 http://的 URL 作为服务器地址）
- `-u` : 在每行包含目标 URL
- `--post` : 使用 POST 而不是 GET（可能会避免缓存）
- `--head` : 使用 HEAD 而不是 GET

### 示例

简单测试：

```
tcping google.com 443
```

持续测试每 5 秒一次：

```
tcping -t -i 5 google.com 80
```

测试 HTTPS 连接，指定超时时间：

```
tcping -w 1 google.com 443
```

使用 HTTP 模式：

```
tcping -h -u google.com
```

## 许可证

本项目基于 GNU 通用公共许可证 v2 发布。

## 鸣谢

原始版本由 Eli Fulkerson 开发，跨平台适配版由社区维护。
