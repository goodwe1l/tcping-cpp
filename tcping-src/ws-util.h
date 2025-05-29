/***********************************************************************
 ws-util.h - 跨平台套接字工具函数声明
***********************************************************************/

#if !defined(WS_UTIL_H)
#define WS_UTIL_H

#include "platform.h"

// 获取套接字错误消息
extern const char* GetSocketErrorMessage(const char* pcMessagePrefix,
        int nErrorID = 0);

// 关闭套接字连接
extern bool ShutdownConnection(socket_t sd);

#ifdef OS_WINDOWS
// 保持原有Windows函数名作为兼容
#define WSAGetLastErrorMessage GetSocketErrorMessage
#endif

#endif // !defined (WS_UTIL_H)

