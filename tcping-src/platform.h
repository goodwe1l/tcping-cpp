#ifndef PLATFORM_H
#define PLATFORM_H

// 平台检测
#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS
#elif defined(__APPLE__)
    #define OS_MAC
    #include <stdio.h> // 添加stdio.h以支持printf函数
#elif defined(__linux__)
    #define OS_LINUX
    #define _POSIX_C_SOURCE 199309L // 启用 clock_gettime
    #include <stdio.h> // 添加stdio.h以支持printf函数
#else
    #error "不支持的平台"
#endif

// 包含必要的头文件
#ifdef OS_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    #include <windows.h>
    typedef SOCKET socket_t;
    typedef int socklen_t;
    #define SOCKET_ERROR_CODE SOCKET_ERROR
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define CLOSE_SOCKET(s) closesocket(s)
    #define LAST_ERROR WSAGetLastError()
    #define EINPROGRESS WSAEINPROGRESS
    #define EWOULDBLOCK WSAEWOULDBLOCK
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h> // For addrinfo
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <sys/time.h>
    #include <time.h> // For clock_gettime and CLOCK_MONOTONIC
    #include <signal.h> // For signal handling
    typedef int socket_t;
    #define SOCKET_ERROR_CODE -1
    #define INVALID_SOCKET_VALUE -1
    #define CLOSE_SOCKET(s) close(s)
    #define LAST_ERROR errno
    #define SD_SEND SHUT_WR
    typedef struct addrinfo ADDRINFO; // Add this for non-Windows
#endif

// 跨平台睡眠函数
#ifdef OS_WINDOWS
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

// 时间相关的跨平台函数
#ifdef OS_WINDOWS
    // Windows已经定义了LARGE_INTEGER等
#else
    typedef struct {
        long long QuadPart;
    } LARGE_INTEGER;
    
    inline void QueryPerformanceCounter(LARGE_INTEGER* pCounter) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        pCounter->QuadPart = (long long)(ts.tv_sec * 1000000000LL + ts.tv_nsec);
    }
    
    inline void QueryPerformanceFrequency(LARGE_INTEGER* pFrequency) {
        pFrequency->QuadPart = 1000000000LL; // 纳秒为单位
    }
#endif

// 蜂鸣声函数
inline void BeepSound(int frequency, int duration) {
#ifdef OS_WINDOWS
    Beep(frequency, duration);
#else
    // Linux/Mac上暂不实现蜂鸣，可以考虑使用printf打印'\a'
    printf("\a");
    (void)frequency; // 避免未使用参数警告
    (void)duration;
#endif
}

// 套接字初始化和清理
#ifdef OS_WINDOWS
    #define INIT_SOCKETS() { WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData); }
    #define CLEANUP_SOCKETS() WSACleanup()
#else
    #define INIT_SOCKETS() {}
    #define CLEANUP_SOCKETS() {}
#endif

// 线程优先级设置
#ifdef OS_WINDOWS
    #define SET_HIGH_PRIORITY() SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)
#else
    #include <sys/resource.h>
    inline void set_process_priority() {
        // 增加进程优先级
        setpriority(PRIO_PROCESS, 0, -10);
    }
    #define SET_HIGH_PRIORITY() set_process_priority()
#endif

// 设置套接字非阻塞
inline int set_nonblocking(socket_t sock) {
#ifdef OS_WINDOWS
    u_long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

#endif // PLATFORM_H
