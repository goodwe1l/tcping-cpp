/***********************************************************************
tcping -- 一个跨平台的TCP探测工具
原始版权: Copyright (C) 2005-2017 Eli Fulkerson
跨平台适配版权: Copyright (C) 2025

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
***********************************************************************/

#include "ws-util.h"
#include "platform.h"

#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>

using namespace std;


//// Constants /////////////////////////////////////////////////////////

const int kBufferSize = 1024;


//// Statics ///////////////////////////////////////////////////////////

// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.
static struct ErrorEntry {
    int nID;
    const char* pcMessage;

    ErrorEntry(int id, const char* pc = 0) :
        nID(id),
        pcMessage(pc) {
    }

    bool operator<(const ErrorEntry& rhs) const {
        return nID < rhs.nID;
    }
} gaErrorList[] = {
    ErrorEntry(0,                  "No error"),
#ifdef OS_WINDOWS
    ErrorEntry(WSAEINTR,           "Interrupted system call"),
    ErrorEntry(WSAEBADF,           "Bad file number"),
    ErrorEntry(WSAEACCES,          "Permission denied"),
    ErrorEntry(WSAEFAULT,          "Bad address"),
    ErrorEntry(WSAEINVAL,          "Invalid argument"),
    ErrorEntry(WSAEMFILE,          "Too many open sockets"),
    ErrorEntry(WSAEWOULDBLOCK,     "Operation would block"),
    ErrorEntry(WSAEINPROGRESS,     "Operation now in progress"),
    ErrorEntry(WSAEALREADY,        "Operation already in progress"),
    ErrorEntry(WSAENOTSOCK,        "Socket operation on non-socket"),
    ErrorEntry(WSAEDESTADDRREQ,    "Destination address required"),
    ErrorEntry(WSAEMSGSIZE,        "Message too long"),
    ErrorEntry(WSAEPROTOTYPE,      "Protocol wrong type for socket"),
    ErrorEntry(WSAENOPROTOOPT,     "Bad protocol option"),
    ErrorEntry(WSAEPROTONOSUPPORT, "Protocol not supported"),
    ErrorEntry(WSAESOCKTNOSUPPORT, "Socket type not supported"),
    ErrorEntry(WSAEOPNOTSUPP,      "Operation not supported on socket"),
    ErrorEntry(WSAEPFNOSUPPORT,    "Protocol family not supported"),
    ErrorEntry(WSAEAFNOSUPPORT,    "Address family not supported"),
    ErrorEntry(WSAEADDRINUSE,      "Address already in use"),
    ErrorEntry(WSAEADDRNOTAVAIL,   "Can't assign requested address"),
    ErrorEntry(WSAENETDOWN,        "Network is down"),
    ErrorEntry(WSAENETUNREACH,     "Network is unreachable"),
    ErrorEntry(WSAENETRESET,       "Net connection reset"),
    ErrorEntry(WSAECONNABORTED,    "Software caused connection abort"),
    ErrorEntry(WSAECONNRESET,      "Connection reset by peer"),
    ErrorEntry(WSAENOBUFS,         "No buffer space available"),
    ErrorEntry(WSAEISCONN,         "Socket is already connected"),
    ErrorEntry(WSAENOTCONN,        "Socket is not connected"),
    ErrorEntry(WSAESHUTDOWN,       "Can't send after socket shutdown"),
    ErrorEntry(WSAETOOMANYREFS,    "Too many references, can't splice"),
    ErrorEntry(WSAETIMEDOUT,       "Connection timed out"),
    ErrorEntry(WSAECONNREFUSED,    "Connection refused"),
    ErrorEntry(WSAELOOP,           "Too many levels of symbolic links"),
    ErrorEntry(WSAENAMETOOLONG,    "File name too long"),
    ErrorEntry(WSAEHOSTDOWN,       "Host is down"),
    ErrorEntry(WSAEHOSTUNREACH,    "No route to host"),
    ErrorEntry(WSAENOTEMPTY,       "Directory not empty"),
    ErrorEntry(WSAEPROCLIM,        "Too many processes"),
    ErrorEntry(WSAEUSERS,          "Too many users"),
    ErrorEntry(WSAEDQUOT,          "Disc quota exceeded"),
    ErrorEntry(WSAESTALE,          "Stale NFS file handle"),
    ErrorEntry(WSAEREMOTE,         "Too many levels of remote in path"),
    ErrorEntry(WSASYSNOTREADY,     "Network system is unavailable"),
    ErrorEntry(WSAVERNOTSUPPORTED, "Winsock version out of range"),
    ErrorEntry(WSANOTINITIALISED,  "WSAStartup not yet called"),
    ErrorEntry(WSAEDISCON,         "Graceful shutdown in progress"),
    ErrorEntry(WSAHOST_NOT_FOUND,  "Host not found"),
    ErrorEntry(WSANO_DATA,         "No host data of that type was found")
#else
    // Unix/Linux/macOS error codes
    ErrorEntry(EINTR,              "Interrupted system call"),
    ErrorEntry(EBADF,              "Bad file number"),
    ErrorEntry(EACCES,             "Permission denied"),
    ErrorEntry(EFAULT,             "Bad address"),
    ErrorEntry(EINVAL,             "Invalid argument"),
    ErrorEntry(EMFILE,             "Too many open files"),
    ErrorEntry(EWOULDBLOCK,        "Operation would block"),
    ErrorEntry(EINPROGRESS,        "Operation now in progress"),
    ErrorEntry(EALREADY,           "Operation already in progress"),
    ErrorEntry(ENOTSOCK,           "Socket operation on non-socket"),
    ErrorEntry(EDESTADDRREQ,       "Destination address required"),
    ErrorEntry(EMSGSIZE,           "Message too long"),
    ErrorEntry(EPROTOTYPE,         "Protocol wrong type for socket"),
    ErrorEntry(ENOPROTOOPT,        "Protocol not available"),
    ErrorEntry(EPROTONOSUPPORT,    "Protocol not supported"),
    ErrorEntry(ESOCKTNOSUPPORT,    "Socket type not supported"),
    ErrorEntry(EOPNOTSUPP,         "Operation not supported"),
    ErrorEntry(EPFNOSUPPORT,       "Protocol family not supported"),
    ErrorEntry(EAFNOSUPPORT,       "Address family not supported"),
    ErrorEntry(EADDRINUSE,         "Address already in use"),
    ErrorEntry(EADDRNOTAVAIL,      "Cannot assign requested address"),
    ErrorEntry(ENETDOWN,           "Network is down"),
    ErrorEntry(ENETUNREACH,        "Network is unreachable"),
    ErrorEntry(ENETRESET,          "Network dropped connection on reset"),
    ErrorEntry(ECONNABORTED,       "Software caused connection abort"),
    ErrorEntry(ECONNRESET,         "Connection reset by peer"),
    ErrorEntry(ENOBUFS,            "No buffer space available"),
    ErrorEntry(EISCONN,            "Transport endpoint is already connected"),
    ErrorEntry(ENOTCONN,           "Transport endpoint is not connected"),
    ErrorEntry(ESHUTDOWN,          "Cannot send after transport endpoint shutdown"),
    ErrorEntry(ETOOMANYREFS,       "Too many references: cannot splice"),
    ErrorEntry(ETIMEDOUT,          "Connection timed out"),
    ErrorEntry(ECONNREFUSED,       "Connection refused"),
    ErrorEntry(ELOOP,              "Too many symbolic links encountered"),
    ErrorEntry(ENAMETOOLONG,       "File name too long"),
    ErrorEntry(EHOSTDOWN,          "Host is down"),
    ErrorEntry(EHOSTUNREACH,       "No route to host"),
    ErrorEntry(ENOTEMPTY,          "Directory not empty")
#endif
};

// 计算错误消息数组的大小
const size_t kNumMessages = sizeof(gaErrorList) / sizeof(gaErrorList[0]);


//// WSAGetLastErrorMessage ////////////////////////////////////////////
// A function similar in spirit to Unix's perror() that tacks a canned
// interpretation of the value of WSAGetLastError() onto the end of a
// passed string, separated by a ": ".  Generally, you should implement
// smarter error handling than this, but for default cases and simple
// programs, this function is sufficient.
//
// This function returns a pointer to an internal static buffer, so you
// must copy the data from this function before you call it again.  It
// follows that this function is also not thread-safe.

const char* GetSocketErrorMessage(const char* pcMessagePrefix,
                                   int nErrorID /* = 0 */) {
    // Build basic error string
    static char acErrorBuffer[256];
    std::ostringstream outs;
    outs << pcMessagePrefix;

#ifdef OS_WINDOWS
    // Windows版本使用现有的错误列表
    // Tack appropriate canned message onto end of supplied message
    // prefix. Note that we do a binary search here: gaErrorList must be
    // sorted by the error constant's value.
    ErrorEntry* pEnd = gaErrorList + kNumMessages;
    ErrorEntry Target(nErrorID ? nErrorID : WSAGetLastError());
    ErrorEntry* it = lower_bound(gaErrorList, pEnd, Target);
    if ((it != pEnd) && (it->nID == Target.nID)) {
        outs << it->pcMessage;
    } else {
        // Didn't find error in list, so make up a generic one
        outs << "unknown error";
    }
    outs << " (" << Target.nID << ")";
#else
    // Linux/Mac版本使用标准错误消息
    int err = nErrorID ? nErrorID : errno;
    outs << strerror(err) << " (" << err << ")";
#endif

    // Finish error message off and return it.
    std::string result = outs.str();
    strncpy(acErrorBuffer, result.c_str(), sizeof(acErrorBuffer) - 1);
    acErrorBuffer[sizeof(acErrorBuffer) - 1] = '\0';
    return acErrorBuffer;
}


//// ShutdownConnection ////////////////////////////////////////////////
// Gracefully shuts the connection sd down.  Returns true if we're
// successful, false otherwise.

bool ShutdownConnection(socket_t sd) {
    // Disallow any further data sends.  This will tell the other side
    // that we want to go away now.  If we skip this step, we don't
    // shut the connection down nicely.
    if (shutdown(sd, SD_SEND) == SOCKET_ERROR_CODE) {
        CLOSE_SOCKET(sd);
        return false;
    }

    // Receive any extra data still sitting on the socket.  After all
    // data is received, this call will block until the remote host
    // acknowledges the TCP control packet sent by the shutdown above.
    // Then we'll get a 0 back from recv, signalling that the remote
    // host has closed its side of the connection.
    char acReadBuffer[kBufferSize];
    while (1) {
        int nNewBytes = recv(sd, acReadBuffer, kBufferSize, 0);
        if (nNewBytes == SOCKET_ERROR_CODE) {
            CLOSE_SOCKET(sd);
            return false;
        } else if (nNewBytes != 0) {
            //    cerr << endl << "FYI, received " << nNewBytes <<
            //            " unexpected bytes during shutdown." << acReadBuffer << endl;
            cout << " (" << nNewBytes << " bytes read)";
        } else {
            // Okay, we're done!
            break;
        }
    }

    // Close the socket.
    if (CLOSE_SOCKET(sd) == SOCKET_ERROR_CODE) {
        return false;
    }

    return true;
}

