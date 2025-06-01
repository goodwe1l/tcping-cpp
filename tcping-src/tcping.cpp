/***********************************************************************
tcping -- 跨平台TCP探测工具
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

#include "platform.h" // 包含跨平台兼容性头文件

#include <iostream>
#include <string.h>
#include <time.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cmath>

#ifndef OS_WINDOWS
#include <signal.h>
#endif

#include "tee.h"
#include "ws-util.h"
#include "tcping.h"
#include "base64.h"

#define NUM_PROBES 4

using namespace std;


// Prototypes

socket_t EstablishConnection(addrinfo *address, int ping_timeout, int force_send_byte, addrinfo *src_address, int &errcode, bool blocking);
socket_t HTTP_EstablishConnection(addrinfo *address, addrinfo *src_address);

void formatIP(std::string &abuffer, addrinfo *address);


LARGE_INTEGER cpu_frequency;
LARGE_INTEGER response_timer1;
LARGE_INTEGER response_timer2;

LARGE_INTEGER http_timer1;
LARGE_INTEGER http_timer2;

int CTRL_C_ABORT;

const int BufferSize = 1024;


bool SendHttp(socket_t sd, char* server, char* document, int http_cmd, int using_proxy, int using_credentials, char* hashed_credentials) {
    char message[1024];
    char cmd[5];

    switch (http_cmd) {
    case HTTP_GET:
#ifdef OS_WINDOWS
		strcpy_s(cmd, sizeof(cmd), "GET");
#else
		strncpy(cmd, "GET", sizeof(cmd) - 1);
		cmd[sizeof(cmd) - 1] = '\0';
#endif
        break;
    case HTTP_HEAD:
#ifdef OS_WINDOWS
		strcpy_s(cmd, sizeof(cmd), "HEAD");
#else
		strncpy(cmd, "HEAD", sizeof(cmd) - 1);
		cmd[sizeof(cmd) - 1] = '\0';
#endif
        break;
    case HTTP_POST:
#ifdef OS_WINDOWS
		strcpy_s(cmd, sizeof(cmd), "POST");
#else
		strncpy(cmd, "POST", sizeof(cmd) - 1);
		cmd[sizeof(cmd) - 1] = '\0';
#endif
        break;
    }    if (document == NULL) {
        document = const_cast<char*>("/");
    }
	if (using_credentials == 0) {
		if (using_proxy == 0) {
#ifdef OS_WINDOWS
			sprintf_s(message, sizeof(message), "%s /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: tcping (cross-platform)\r\n\r\n", cmd, document, server);
#else
			snprintf(message, sizeof(message), "%s /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: tcping (cross-platform)\r\n\r\n", cmd, document, server);
#endif
		}
		else {
#ifdef OS_WINDOWS
			sprintf_s(message, sizeof(message), "%s http://%s/%s HTTP/1.1\r\nConnection: close\r\nUser-Agent: tcping (cross-platform)\r\n\r\n", cmd, server, document);
#else
			snprintf(message, sizeof(message), "%s http://%s/%s HTTP/1.1\r\nConnection: close\r\nUser-Agent: tcping (cross-platform)\r\n\r\n", cmd, server, document);
#endif
		}
	}
	else {
		if (using_proxy == 0) {
#ifdef OS_WINDOWS
			sprintf_s(message, sizeof(message), "%s /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: tcping (cross-platform)\r\nProxy-Authorization: Basic %s\r\n\r\n", cmd, document, server, hashed_credentials);
#else
			snprintf(message, sizeof(message), "%s /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: tcping (cross-platform)\r\nProxy-Authorization: Basic %s\r\n\r\n", cmd, document, server, hashed_credentials);
#endif
		}
		else {
#ifdef OS_WINDOWS
			sprintf_s(message, sizeof(message), "%s http://%s/%s HTTP/1.1\r\nConnection: close\r\nUser-Agent: tcping (cross-platform)\r\nProxy-Authorization: Basic %s\r\n\r\n", cmd, server, document, hashed_credentials);
#else
			snprintf(message, sizeof(message), "%s http://%s/%s HTTP/1.1\r\nConnection: close\r\nUser-Agent: tcping (cross-platform)\r\nProxy-Authorization: Basic %s\r\n\r\n", cmd, server, document, hashed_credentials);
#endif
		}
	}

    const int messageLen = (int)strlen(message);
	
    // Send the string to the server
    if (send(sd, message, messageLen, 0) != SOCKET_ERROR_CODE) {
        return true;
    } else {
        return false;
    }
}


//// ReadReply /////////////////////////////////////////////////////////
// Read the reply packet and check it for sanity.  Returns -1 on
// error, 0 on connection closed, > 0 on success.

int ReadReply(socket_t sd, int &bytes_received, int &http_status) {
    // Read reply from server
    char acReadBuffer[BufferSize];
    char acTrashBuffer[BufferSize];

    int nTotalBytes = 0;
    int nNewBytes = 0;
    while (1) {

        if (nTotalBytes < BufferSize) {
            nNewBytes = recv(sd, acReadBuffer + nTotalBytes, BufferSize - nTotalBytes, 0);
        } else {
            nNewBytes = recv(sd, acTrashBuffer, BufferSize, 0);
        }

        if (nNewBytes == SOCKET_ERROR_CODE) {
            return -1;
        } else if (nNewBytes == 0) {
            break;
        }

        nTotalBytes += nNewBytes;
    }

    bytes_received =  nTotalBytes;

    //parse out the http status from the first line of the response
    char* statusptr;
    char* tmpptr;

    // hop over the initial "HTTP/1.1"
    statusptr = strchr(acReadBuffer, ' ');
    ++statusptr;
    tmpptr = strchr(statusptr, ' ');

    // should be at the " " past the error code now
    *tmpptr = '\0';

    http_status = atoi(statusptr);
    return 0;
}

void controlc() {
    if (CTRL_C_ABORT == 1) {
        cout.flush();
        cout << "Wow, you really mean it.  I'm sorry... I'll stop now. :(" << endl;
        exit(1);
    }
    cout << "Control-C" << endl;
    CTRL_C_ABORT = 1;
}

void COLOR_RESET(int use_color) {
	if (use_color == 1) {
		printf("%c[%dm", 0x1B, 0);
	}
#ifdef OS_WINDOWS
	if (use_color == 2) {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, 7);
	}
#endif
}

void COLOR_RED(int use_color) {
	if (use_color == 1) {
		printf("%c[%dm", 0x1B, 31);
	}
#ifdef OS_WINDOWS
	if (use_color == 2) {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, 12);
	}
#endif
}


int DoWinsock(const char* pcHost, int nPort, int times_to_ping, double ping_interval, int include_timestamp, int beep_mode, int ping_timeout, int relookup_interval, int auto_exit_on_success, int force_send_byte, int include_url, int use_http, char* docptr, int http_cmd, int include_jitter, int jitter_sample_size, char* logfile, int use_logfile, int ipv, char* proxy_server, int proxy_port, int using_credentials, char* proxy_credentials, int only_changes, int no_statistics, int giveup_count, class tee &out, int use_source_address, const char *src_address, bool blocking, int always_print_domain, int use_color) {

	COLOR_RESET(use_color);
	
	char web_server[2048];
	int using_proxy = 0;
	// if we are using a http proxy server, the pcHost needs to be the server address 
    if (proxy_server == nullptr || proxy_server[0] == 0) {
#ifdef OS_WINDOWS
		sprintf_s(web_server, sizeof(web_server), "%s", pcHost);
#else
		snprintf(web_server, sizeof(web_server), "%s", pcHost);
#endif
		//web_server[0] = 0;
	} else {
#ifdef OS_WINDOWS		
		sprintf_s(web_server, sizeof(web_server), "%s", proxy_server); // Use web_server for proxy
#else
		snprintf(web_server, sizeof(web_server), "%s", proxy_server); // Use web_server for proxy
#endif
		//@@ 使用安全的字符串函数
		// pcHost is const, cannot be modified here. The actual target host is in original pcHost.
		// The connection will be made to proxy_server (now in web_server)
		// and the HTTP request will contain the original pcHost.
		nPort = proxy_port;
		using_proxy = 1;
	}

	char hashed_credentials[2048] = {0}; // Initialize to empty string
	if (using_credentials == 1 && proxy_credentials != nullptr) {
		const std::string s(proxy_credentials);
		std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
#ifdef OS_WINDOWS
		sprintf_s(hashed_credentials, sizeof(hashed_credentials), "%s", encoded.c_str());
#else
		snprintf(hashed_credentials, sizeof(hashed_credentials), "%s", encoded.c_str());
#endif
	}

#ifdef OS_WINDOWS
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)&controlc, TRUE);
#else
    signal(SIGINT, [](int signum) { controlc(); });
#endif
    CTRL_C_ABORT = 0;

    int bytes_received = 0;
    int http_status = 0;

    double bps = 0;

    int number_of_pings = 0;	// total number of tcpings issued
    double running_total_ms = 0;	// running total of values of pings... divide by number_of_pings for avg
    double lowest_ping = 50000;		// lowest ping in series ... starts high so it will drop
    double max_ping = 0;			// highest ping in series

    double running_total_ms_http = 0;
    double lowest_ping_http = 50000;
    double max_ping_http = 0;

    int success_counter = 0;
    int failure_counter = 0;
    int deferred_counter = 0;

	int sequential_failure_counter = 0;

    int loopcounter = 0;			// number of probes to send

    // Timestamp variablees
	time_t rawtime;
	//struct tm * timeinfo;
	struct tm  timeinfo;
	char dateStr[11];
	char timeStr[9];

    int beep_flag = -1;  // 0 for we're down, 1 for we're up
    double response_time;
    double http_response_time;

    int have_valid_target = 1;    // jitter rolling average - 抖动滚动平均值计算
    // 创建指定大小的缓冲区来存储TCP连接响应时间，用于计算抖动
    // Ensure jitter_sample_size is valid before creating vectors
    if (jitter_sample_size <= 0) {
        jitter_sample_size = 10; // Default to a small size if invalid
    }
    vector<double> jitterbuffer(jitter_sample_size);     // TCP连接响应时间缓冲区
    vector<double> http_jitterbuffer(jitter_sample_size); // HTTP响应时间缓冲区

    int jitterpos = 0;      // 缓冲区当前位置指针，用于循环写入新的响应时间
    int success_flag = 0;   // 记录当前周期是否成功的标志，用于抖动滚动平均值计算
    double j;               // 临时变量，用于计算抖动平均值

	// 使用绝对值(abs)计算抖动，因为负数会影响最大/最小值的计算
	// 抖动相关变量 - 连接建立时间
	double current_jitter = 0;            // 当前抖动值
	double running_total_abs_jitter = 0;  // 累计绝对抖动值总和
	double lowest_abs_jitter = 50000;     // 最小绝对抖动值（初始设为一个大数）
	double max_abs_jitter = 0;            // 最大绝对抖动值
	
	// HTTP响应抖动相关变量 - 内容下载时间
	double current_http_jitter = 0;            // 当前HTTP抖动值
	double running_total_abs_http_jitter = 0;  // 累计HTTP绝对抖动值总和
	double lowest_abs_http_jitter = 50000;     // 最小HTTP绝对抖动值（初始设为一个大数）
	double max_abs_http_jitter = 0;            // 最大HTTP绝对抖动值




	bool last_cycle_success = false;
	int number_same_cycles = 0;

	addrinfo hint, *AddrInfo, *AI;
	char p[6];
	int r;
	int found;
#ifdef OS_WINDOWS
	sprintf_s(p, sizeof(p), "%d", nPort);
#else
	snprintf(p, sizeof(p), "%d", nPort);
#endif
    memset(&hint, 0, sizeof (hint));
    hint.ai_family = PF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;



	
	
	
	// Find the server's address	
	r = getaddrinfo(pcHost, p, &hint, &AddrInfo);
    
	if (r != 0 ) {
		
		if (relookup_interval == -1) {
			COLOR_RED(use_color);
			out.pf("DNS: Could not find host - %s, aborting\n", pcHost);
			COLOR_RESET(use_color);
			return 3;
		}
		else {
			have_valid_target = 0;
		}
	}
	found = 0;
	for (AI = AddrInfo; AI != NULL; AI = AI->ai_next) {
		if ((AI->ai_family == AF_UNSPEC && ipv == 0) ||
			(AI->ai_family == AF_INET && ipv != 6) ||
			(AI->ai_family == AF_INET6 && ipv != 4)) {
			found = 1;
			break;
		}
	}
	if (found == 0) {
		if (relookup_interval == -1) {
			COLOR_RED(use_color);
			out.pf("DNS: No valid host found in AddrInfo for that type\n");
			COLOR_RESET(use_color);
			return 3;
		}
		else {
			have_valid_target = 0;
		}
	}
		// source IP
	addrinfo *SRCAI = NULL;

	if (use_source_address != 0) {
		r = getaddrinfo(src_address, NULL, NULL, &SRCAI);
		
		if (r != 0) {
			COLOR_RED(use_color);
			out.pf("-S:  You specified '%s' as a source address, couldn't do anything with that, aborting.\n", src_address);
			COLOR_RESET(use_color);
			return 4;
		}
				socket_t sd = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);
		
		r = ::bind(sd, SRCAI->ai_addr, (int)SRCAI->ai_addrlen);

		if (r == SOCKET_ERROR_CODE) {
			COLOR_RED(use_color);
			out.pf("Binding local source address '%s' failed with error %u\n", src_address, LAST_ERROR);
			COLOR_RESET(use_color);
			return 5;
		}


	}

	int errorcode_stash = 0;

    while ((loopcounter < times_to_ping || times_to_ping == -1) && CTRL_C_ABORT == 0 ) {

        success_flag = 0;

        if (((number_of_pings % relookup_interval == 0) && (relookup_interval != -1) && number_of_pings > 0) || have_valid_target == 0) {
			//freeaddrinfo(AddrInfo);   // freeing from the previous cycle @@ don't know if this works this thing still seems to leak
			// Find the server's address
			// Duplicate code here because dealing with resource leaks, getaddrinfo and the
			// differing IPV4 vs IPV6 structures was just obnoxious.
			found = 0;
			r = getaddrinfo(pcHost, p, &hint, &AddrInfo);

			if (r != 0) {
				out.pf("DNS: Could not find host - %s\n", pcHost);
				have_valid_target = 0;
			}
			for (AI = AddrInfo; AI !=NULL; AI=AI->ai_next) {
				if ( (AI->ai_family == AF_UNSPEC && ipv == 0) ||
				     (AI->ai_family == AF_INET && ipv != 6) ||
				     (AI->ai_family == AF_INET6 && ipv != 4) ) {
				
					have_valid_target = 1;
					found = 1;
					std::string abuffer;
					formatIP(abuffer, AI);
					out.pf("DNS: %s is %s\n", pcHost, abuffer.c_str());
					break;
				}
				if (found == 0) {
					out.pf("DNS: No valid host found in AddrInfo for that type\n");
				}	 
			}
        }

        if (include_timestamp == 1) {
#ifdef OS_WINDOWS
			errno_t err;
            _strtime_s(timeStr, sizeof(timeStr));
            time ( &rawtime );
            err = localtime_s(&timeinfo, &rawtime);
            strftime(dateStr, 11, "%Y:%m:%d",&timeinfo);
#else
            time ( &rawtime );
            localtime_r(&rawtime, &timeinfo); // POSIX compliant
            strftime(dateStr, sizeof(dateStr), "%Y:%m:%d", &timeinfo);
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
#endif
            out.pf("%s %s ", dateStr, timeStr);
        }

        if (have_valid_target == 1) {

            socket_t sd;

#ifdef OS_WINDOWS
            // apparently... QueryPerformanceCounter isn't thread safe unless we do this
            SetThreadAffinityMask(GetCurrentThread(),1);
#endif

            // start the timer right before we do the connection
            QueryPerformanceFrequency((LARGE_INTEGER *)&cpu_frequency);
            QueryPerformanceCounter((LARGE_INTEGER *) &response_timer1);

            // Connect to the server
            if (use_http == 0) {
                sd = EstablishConnection(AI, ping_timeout, force_send_byte, SRCAI, errorcode_stash, blocking);
            } else {
                sd = HTTP_EstablishConnection(AI, SRCAI);
            }

            // grab the timeout as early as possible
            QueryPerformanceCounter((LARGE_INTEGER *) &response_timer2);

            if (sd == INVALID_SOCKET_VALUE) {
			
				if (only_changes == 1) {
					if (last_cycle_success == false) 
					{
						// no change, so kill the output
						out.enable(false);
						number_same_cycles += 1;
					} else {
						out.enable(true);
						if (number_of_pings > 0) {
							out.pf("(%d successful)\\n", number_same_cycles);
						}
						number_same_cycles = 0;
					}
					last_cycle_success = false;
				}
				std::string abuffer;
				formatIP(abuffer, AI);
				
				COLOR_RED(use_color);
				if (always_print_domain == 0) {
					out.pf("Probing %s:%d/tcp - ", abuffer.c_str(), nPort);
				}
				else {
					out.pf("Probing %s:%d/tcp - ", pcHost, nPort);
				}

				//if (WSAGetLastError() != 0) {
				if (errorcode_stash != 0) {
#ifdef OS_WINDOWS
					out.p( WSAGetLastErrorMessage("",errorcode_stash));
#else
                    out.p(strerror(errorcode_stash));
#endif
					
				} else {
					out.p( "No response" );
				}
                failure_counter++;
				sequential_failure_counter++;                if (beep_mode == 4 || beep_mode == 1 || (beep_mode == 3 && beep_flag == 1)) {
                    // 连接失败时发出两次蜂鸣声
                    BeepSound(1000, 300);
                    BeepSound(1000, 300);
                    cout << " " << "*" << "*";
                }
                beep_flag = 0;
            } else {
			
				if (only_changes == 1) {
					if (last_cycle_success == true) 
					{
						// no change, so kill the output
						
						out.enable(false);
						number_same_cycles += 1;
					} else {
						out.enable(true);
						if (number_of_pings > 0) {
							out.pf("(%d unsuccessful)\n", number_same_cycles);
						}

						number_same_cycles = 0;
					}
					last_cycle_success = true;
					sequential_failure_counter = 0;
				}
				
				std::string abuffer;
				formatIP(abuffer, AI);

				if (always_print_domain == 0) {	
					out.pf("Probing %s:%d/tcp - ", abuffer.c_str(), nPort);
				}
				else {
					out.pf("Probing %s:%d/tcp - ", pcHost, nPort);
				}

                if (use_http == 0) {
                    out.p("Port is open");
                    success_counter++;
                    success_flag = 1;
                } else {
                    // consider only incrementing if http response @@
                    out.p("HTTP is open");
                    success_counter++;
                    success_flag = 1;

                    // send http send/response
#ifdef OS_WINDOWS
                    SetThreadAffinityMask(GetCurrentThread(),1);
#endif

                    QueryPerformanceFrequency((LARGE_INTEGER *)&cpu_frequency);
                    QueryPerformanceCounter((LARGE_INTEGER *) &http_timer1);

                    SendHttp(sd, web_server, docptr, http_cmd, using_proxy, using_credentials, hashed_credentials);
                    ReadReply(sd, bytes_received, http_status);
                    QueryPerformanceCounter((LARGE_INTEGER *) &http_timer2);
					CLOSE_SOCKET(sd);
                }                if (beep_mode == 4 || beep_mode == 2 || (beep_mode == 3 && beep_flag == 0)) {
                    // 连接成功时发出一次蜂鸣声
                    BeepSound(1000, 300);
                    cout << " *";
                }
                beep_flag = 1;
            }
            // Shut connection down
            if (ShutdownConnection(sd)) {
                // room here for connection shutdown success check...
            } else {
                // room here for connection shutdown failure check...
            }

            response_time=( (double) ( (response_timer2.QuadPart - response_timer1.QuadPart) * (double) 1000.0 / (double) cpu_frequency.QuadPart) );
            http_response_time=( (double) ( (http_timer2.QuadPart - http_timer1.QuadPart) * (double) 1000.0 / (double) cpu_frequency.QuadPart) );

            out.pf( " - time=%0.3fms ", response_time);

            if (use_http == 1) {
                if (include_url == 1) {
                    if (docptr != NULL) {
                        out.pf( "page:http://%s/%s ",pcHost,docptr);
                    } else {
                        out.pf( "page:http://%s ", pcHost);
                    }
                }

                out.pf("rcv_time=%0.3f status=%d bytes=%d ", http_response_time, http_status, bytes_received);

                bps = bytes_received * 1000 / http_response_time;
                bps = bps * 8 / 1000;

                out.pf("kbit/s=~%0.3f ",bps);
            }


            // Calculate the statistics...
            number_of_pings++;

            if (sd != INVALID_SOCKET_VALUE) {
                running_total_ms += response_time;

                if (response_time < lowest_ping) {
                    lowest_ping = response_time;
                }

                if (response_time > max_ping) {
                    max_ping = response_time;
                }

                if (use_http == 1) {

                    running_total_ms_http += http_response_time;

                    if (http_response_time < lowest_ping_http) {
                        lowest_ping_http = http_response_time;
                    }

                    if (http_response_time > max_ping_http) {
                        max_ping_http = http_response_time;
                    }
                }
            }            /*
              Two ways to measure jitter.  If jitter_sample_size == 0, then its a total/times, non inclusive of the current go.
              Otherwise, we calculate it based on the prior [jitter_sample_size] values, non inclusive.
              
              抖动测量有两种方式：
              1. 如果 jitter_sample_size == 0，则使用"总响应时间/总次数"的方式计算历史平均值，当前值不包含在历史平均值中
              2. 否则，基于先前的 [jitter_sample_size] 个值计算滚动平均值（不包括当前值）
             */

            if (include_jitter == 1 && success_counter > 1) {
                if (jitter_sample_size == 0) {
                    // 未指定样本大小，不使用滚动平均值，而是使用所有历史数据计算抖动
                    
                    // 计算当前抖动值 = 当前响应时间 - 之前所有响应时间的平均值
                    // (running_total_ms - response_time) 移除当前值，只计算历史值
                    // (success_counter - 1) 排除当前成功次数
					current_jitter = response_time - ((running_total_ms - response_time) / (success_counter - 1));

					// 输出抖动值
					out.pf("jitter=%0.3f ", current_jitter);

					// 更新最大抖动值（绝对值）
					if (max_abs_jitter < abs(current_jitter)) {
						max_abs_jitter = abs(current_jitter);
					}

					// 更新最小抖动值（绝对值）
					if (lowest_abs_jitter > abs(current_jitter)) {
						lowest_abs_jitter = abs(current_jitter);
					}

					// 累加绝对抖动值，用于计算平均抖动
					running_total_abs_jitter += abs(current_jitter);                    if (use_http == 1) {
                        // 如果使用HTTP模式，需要额外计算HTTP响应的抖动值
                        
                        // 计算HTTP响应的抖动值 = 当前HTTP响应时间 - 之前所有HTTP响应时间的平均值
						current_http_jitter = http_response_time - ((running_total_ms_http - http_response_time) / (success_counter - 1));
						
						// 输出HTTP响应抖动值
						out.pf("rcv_jitter=%0.3f ", current_http_jitter);

						// 更新HTTP响应的最大抖动值（绝对值）
						if (max_abs_http_jitter < abs(current_http_jitter)) {
							max_abs_http_jitter = abs(current_http_jitter);
						}

						// 更新HTTP响应的最小抖动值（绝对值）
						if (lowest_abs_http_jitter > abs(current_http_jitter)) {
							lowest_abs_http_jitter = abs(current_http_jitter);
						}

						// 累加HTTP响应的绝对抖动值，用于计算平均抖动
						running_total_abs_http_jitter += abs(current_http_jitter);
                    }                } else {
                    // 使用滚动窗口计算抖动值（基于最近的jitter_sample_size个样本）
                    
                    // 初始化累计响应时间
                    j = 0;

                    // 计算滑动窗口内所有响应时间的总和
                    // 样本窗口大小取jitter_sample_size和(success_counter-1)的较小值
                    for (int x=0; x< min(jitter_sample_size, success_counter - 1); x++) {
                        j = j + jitterbuffer[x];
                    }
                    
                    // 计算当前抖动值 = 当前响应时间 - 滑动窗口内响应时间的平均值
					current_jitter = response_time - (j / min(success_counter - 1, jitter_sample_size));
					
					// 输出抖动值
					out.pf("jitter=%0.3f ", current_jitter);

					// 更新最大抖动值（绝对值）
					if (max_abs_jitter < abs(current_jitter)) {
						max_abs_jitter = abs(current_jitter);
					}

					// 更新最小抖动值（绝对值）
					if (lowest_abs_jitter > abs(current_jitter)) {
						lowest_abs_jitter = abs(current_jitter);
					}

					// 累加绝对抖动值，用于计算平均抖动
					running_total_abs_jitter += abs(current_jitter);                    if (use_http == 1) {
                        // 如果使用HTTP模式，需要使用滚动窗口计算HTTP响应的抖动值
                        
                        // 初始化累计HTTP响应时间
                        j = 0;
                        
                        // 计算滑动窗口内所有HTTP响应时间的总和
                        for (int x=0; x< min(jitter_sample_size, success_counter - 1); x++) {
                            j = j + http_jitterbuffer[x];
                        }
                        
                        // 计算当前HTTP抖动值 = 当前HTTP响应时间 - 滑动窗口内HTTP响应时间的平均值
						current_http_jitter = http_response_time - (j / min(success_counter - 1, jitter_sample_size));
						
						// 输出HTTP响应抖动值
						out.pf("rcv_jitter=%0.3f ", current_http_jitter);						// 更新HTTP响应的最大抖动值（绝对值）
						if (max_abs_http_jitter < abs(current_http_jitter)) {
							max_abs_http_jitter = abs(current_http_jitter);
						}

						// 更新HTTP响应的最小抖动值（绝对值）
						if (lowest_abs_http_jitter > abs(current_http_jitter)) {
							lowest_abs_http_jitter = abs(current_http_jitter);
						}

						// 累加HTTP响应的绝对抖动值，用于计算平均抖动
						running_total_abs_http_jitter += abs(current_http_jitter);
                    }
                }
            }            if (success_flag == 1 && jitter_sample_size > 0) {
                // 更新抖动缓冲区，仅在连接成功且设置了滑动窗口大小时进行
                jitterbuffer[jitterpos] = response_time;           // 存储当前TCP连接响应时间
                http_jitterbuffer[jitterpos] = http_response_time; // 存储当前HTTP响应时间
                jitterpos++;                                       // 移动到下一个缓冲区位置

                // 简单的滚动平均实现 - 当缓冲区填满时循环回到数组开始位置
                // 这样可以保持一个固定大小的滑动窗口，始终记录最近的N个样本
                if (jitterpos == jitter_sample_size) {
                    jitterpos = 0;
                }
            }


			COLOR_RESET(use_color);
            out.p("\n");


            loopcounter++;
            if ((loopcounter == times_to_ping) || ((auto_exit_on_success == 1) && (success_counter > 0))) {
                break;
            }

			if (sequential_failure_counter >= giveup_count && giveup_count != 0) {
				break;
			}

        } else {
            // no valid target
            response_time = 0;
            deferred_counter++;
            out.p("No host to ping.\n");
        }

        int zzz = 0;
        double wakeup = (ping_interval * 1000) - response_time;
        if (wakeup > 0 ) {
            while (zzz < wakeup && CTRL_C_ABORT ==0) {
                SLEEP_MS(10);
                zzz += 10;
            }
        }
    }

	out.enable(true);

	if (!no_statistics) {

		std::string abuffer;
		if (have_valid_target == 1) {
			formatIP(abuffer, AI);
		}
		else {
			// if we have a bouncing DNS host, we don't have an IP to format correctly, so just spit out what they gave us as an argument...
			abuffer = pcHost;
		}
		
		out.pf("\nPing statistics for %s:%d\n", abuffer.c_str(), nPort);
		out.pf("     %d probes sent. \n", number_of_pings);

		float fail_percent = 100 * (float)failure_counter / ((float)success_counter + (float)failure_counter);

		// What is this?  Its quadruple %%%% because we are passing through printf *twice*
		out.pf("     %d successful, %d failed.  (%0.2f%%%% fail)\n", success_counter, failure_counter, fail_percent);
		
		if (deferred_counter > 0) {

			out.pf("     %d skipped due to failed DNS lookup.\n", deferred_counter);
		}
		if (success_counter > 0) {
			if (failure_counter > 0) {
				out.p("Approximate trip times in milli-seconds (successful connections only):\n");
			}
			else {
				out.p("Approximate trip times in milli-seconds:\n");
			}

			out.pf("     Minimum = %0.3fms, Maximum = %0.3fms, Average = %0.3fms\n", lowest_ping, max_ping, running_total_ms / success_counter);

			if (use_http == 1) {
				out.p("Approximate download times in milli-seconds:\n");
				out.pf("     Minimum = %0.3fms, Maximum = %0.3fms, Average = %0.3fms\n", lowest_ping_http, max_ping_http, running_total_ms_http / success_counter);
			}			// 如果启用了抖动计算且有超过1次的成功连接（抖动计算需要至少2次成功连接）
			if (include_jitter && success_counter > 1) {
				out.p("Jitter:\n"); // 输出TCP连接抖动统计信息标题
				// 输出抖动的最小值、最大值和平均值
				// 注意：平均值计算中使用(success_counter - 1)是因为抖动计算在第一次成功连接后才开始
				out.pf("     Minimum = %0.3fms, Maximum = %0.3fms, Average = %0.3fms\n", 
					lowest_abs_jitter, max_abs_jitter, running_total_abs_jitter / (success_counter - 1));
				
				// 如果启用了HTTP模式，还需要输出HTTP响应抖动统计信息
				if (use_http) {
					out.p("HTTP response jitter:\n"); // 输出HTTP响应抖动统计信息标题
					// 输出HTTP响应抖动的最小值、最大值和平均值
					out.pf("     Minimum = %0.3fms, Maximum = %0.3fms, Average = %0.3fms\n", 
						lowest_abs_http_jitter, max_abs_http_jitter, running_total_abs_http_jitter / (success_counter - 1));
				}
			}
		}
		else {
			out.p("Was unable to connect, cannot provide trip statistics.\n");
		}
	}
	freeaddrinfo(AddrInfo);
	
    // report our total, abject failure.
    if (success_counter == 0) {
        return 1;
    }

    // return our intermittent failure
    if (success_counter > 0 && failure_counter > 0) {
        return 2;
    }

    return 0;
}

//// EstablishConnection ///////////////////////////////////////////////
// Connects to a given address, on a given port, both of which must be
// in network byte order.  Returns newly-connected socket if we succeed,
// or INVALID_SOCKET_VALUE if we fail.

socket_t EstablishConnection(addrinfo* address, int ping_timeout, int force_send_byte, addrinfo* src_address, int &errorcode, bool blocking) {

    LARGE_INTEGER timer1;
    LARGE_INTEGER timer2;
    LARGE_INTEGER cpu_freq;

    double time_so_far;

	

    // Create a stream socket
    socket_t sd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled;
    // If iMode != 0, non-blocking mode is enabled.
#ifdef OS_WINDOWS
    u_long iMode = 1;
#else
    int iMode = 1;
#endif
	// ok - the -w option, if enabled, will make this other than -1.  So this leaves us on blocking mode
	// ... which will just use the default timeout length.  BUT, since its blocking mode, we can get a useful
	// result from the connect() function, which means we can detect 10061 - connection refused vs a timeout.
	if (blocking == true) {
		//cout << " no timeout @@ " << endl;
		iMode = 0;
	}
	//u_long iMode = 0;
#ifdef OS_WINDOWS
    ioctlsocket(sd, FIONBIO, &iMode);
#else    // 在Unix系统中设置非阻塞模式
    if (iMode != 0) {
        set_nonblocking(sd);
    }
#endif

	if (src_address != NULL) {
		::bind(sd, src_address->ai_addr, (int)src_address->ai_addrlen);
		// temporary
		//if (r == SOCKET_ERROR) {
		//	wprintf(L"bind failed with error %u\n", WSAGetLastError());
		//}
		//else {
		//	wprintf(L"bind returned success\n");
		//}
	}

    QueryPerformanceFrequency((LARGE_INTEGER *)&cpu_freq);
    QueryPerformanceCounter((LARGE_INTEGER *) &timer1);
	
	int conResult = -999;
	conResult = connect(sd, address->ai_addr, (int)address->ai_addrlen);
	
	if (conResult == SOCKET_ERROR_CODE && iMode == 0) { // Use SOCKET_ERROR_CODE
		//cout << "result " << conResult << " current error: " << WSAGetLastError() << endl;
		errorcode = LAST_ERROR; // Use LAST_ERROR
		CLOSE_SOCKET(sd); // Use CLOSE_SOCKET
		return INVALID_SOCKET_VALUE; // Use INVALID_SOCKET_VALUE
	}
	
    char sendy[] = "."; // Make it an array to avoid const char* to char* warning if compiler is strict
    int size = 1;
    int sendstatus = 1000;

    bool done = false;

    while (!done && !CTRL_C_ABORT) {

        if (force_send_byte == 0) {
            sendstatus = send(sd, NULL, 0, 0);   // should return 0 below
        } else {
            sendstatus = send(sd, sendy, size, 0);   // should return sizeof(sendy) below
        }


        // one error code is if you send a send of size 0, the other is if you actually send data.
        if (sendstatus == size && force_send_byte == 1) {
			CLOSE_SOCKET(sd); // Use CLOSE_SOCKET
			errorcode = LAST_ERROR; // Use LAST_ERROR
            return sd;
        }

        if (sendstatus == 0 && force_send_byte == 0) {
			CLOSE_SOCKET(sd); // Use CLOSE_SOCKET
			errorcode = LAST_ERROR; // Use LAST_ERROR
            return sd;
        }

        QueryPerformanceCounter((LARGE_INTEGER *) &timer2);

        time_so_far = ( (double) ( (timer2.QuadPart - timer1.QuadPart) * (double) 1000.0 / (double) cpu_freq.QuadPart) );

        if (time_so_far >= ping_timeout) {
            done = true;
        } else {
			if (time_so_far < 200) {  // the idea here is to not grind the processor too hard if the precision gained isn't going to be useful.
				SLEEP_MS(0); // Use SLEEP_MS
			} else {
				SLEEP_MS(1); // Use SLEEP_MS
			}
        }
    }
	
	CLOSE_SOCKET(sd); // Use CLOSE_SOCKET
	errorcode = LAST_ERROR; // Use LAST_ERROR
    return INVALID_SOCKET_VALUE; // Use INVALID_SOCKET_VALUE
}

socket_t HTTP_EstablishConnection(addrinfo* address, addrinfo* src_address) {

    // Create a stream socket

	socket_t sd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    // would be nice to limit the huge timeout in cases where the tcp connection times out (as opposed
    // to bouncing off a closed port, but this stuff doesn't work for some reason.
    /*int timeout = 10;
    int tosize = sizeof(timeout);    setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO , (char*)&timeout, (int)&tosize);
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO , (char*)&timeout, (int)&tosize);
    */
	if (src_address != NULL) {
		::bind(sd, src_address->ai_addr, (int)src_address->ai_addrlen);
		// temporary
		//if (r == SOCKET_ERROR) {
//			wprintf(L"bind failed with error %u\n", WSAGetLastError());
		//}
		//else {
//			wprintf(L"bind returned success\n");
		//}
	}

    if (sd != INVALID_SOCKET_VALUE) {

        if (connect(sd, address->ai_addr, (int)address->ai_addrlen) == SOCKET_ERROR_CODE) { // Use SOCKET_ERROR_CODE
            sd = INVALID_SOCKET_VALUE; // Use INVALID_SOCKET_VALUE
        }
    }

    return sd;
}


void formatIP(std::string &abuffer, addrinfo* address) {

	char buffer[46];
	int ret;

	switch (address->ai_family) {
	case AF_INET:
		struct sockaddr_in  *sockaddr_ipv4;
		sockaddr_ipv4 = (struct sockaddr_in *) address->ai_addr;
		//sprintf(buffer, inet_ntoa(sockaddr_ipv4->sin_addr));
#ifdef OS_WINDOWS
		sprintf_s(buffer, sizeof(buffer), "%s", inet_ntoa(sockaddr_ipv4->sin_addr));
#else
		snprintf(buffer, sizeof(buffer), "%s", inet_ntoa(sockaddr_ipv4->sin_addr));
#endif
		break;	case AF_INET6:
		// inet_ntop is not available on XP, do not use
		//inet_ntop(address->ai_family, address->ai_addr, buffer, bufferlen);

		ret = getnameinfo(address->ai_addr, (int)address->ai_addrlen, buffer, sizeof (buffer), NULL, 0, NI_NUMERICHOST);
        (void)ret; // Suppress unused variable warning

		break;
	}
	
	abuffer = buffer;
	
}


int DoWinsock_Single(const char* pcHost, int nPort, int times_to_ping, double ping_interval, int include_timestamp, int beep_mode, int ping_timeout, int relookup_interval, int auto_exit_on_success, int force_send_byte, int include_url, int use_http, char* docptr, int http_cmd, int include_jitter, int jitter_sample_size, char* logfile, int use_logfile, int ipv, char* proxy_server, int proxy_port, int using_credentials, char* proxy_credentials, int only_changes, int no_statistics, int giveup_count, class tee &out, int use_source_address, const char *src_address, bool blocking, int always_print_domain, int use_color) {
	
	int retval;
	
	retval = DoWinsock(pcHost, nPort, times_to_ping, ping_interval, include_timestamp, beep_mode, ping_timeout, relookup_interval, auto_exit_on_success, force_send_byte, include_url, use_http, docptr, http_cmd, include_jitter, jitter_sample_size, logfile, use_logfile, ipv, proxy_server, proxy_port, using_credentials, proxy_credentials, only_changes, no_statistics, giveup_count, out, use_source_address, src_address, blocking, always_print_domain, use_color);
	return retval;
}

int DoWinsock_Multi(const char* pcHost, int nPort, int times_to_ping, double ping_interval, int include_timestamp, int beep_mode, int ping_timeout, int relookup_interval, int auto_exit_on_success, int force_send_byte, int include_url, int use_http, char* docptr, int http_cmd, int include_jitter, int jitter_sample_size, char* logfile, int use_logfile, int ipv, char* proxy_server, int proxy_port, int using_credentials, char* proxy_credentials, int only_changes, int no_statistics, int giveup_count, int file_times_to_loop, char* urlfile, class tee &out, int use_source_address, const char *src_address, bool blocking, int always_print_domain, int use_color) {
	
	int retval;


	int count = 0;
	while (count < file_times_to_loop || file_times_to_loop == -1) {
		std::ifstream input(urlfile);
		std::string line;

		//while (std::getline(input, line))
		while (std::getline(input, line))
		{
			std::stringstream ss(line);
			std::string line_ip;
			int line_port;

			if (ss >> line_ip) {
				if (ss >> line_port) {
					//out.p("success");
				}
				else {
					line_port = nPort;
				}
			}
			else {
				break;
			}			char pcHost_f[255];
			//strcpy_s(pcHost_f, sizeof(pcHost_f), line.c_str());
#ifdef OS_WINDOWS
			strcpy_s(pcHost_f, sizeof(pcHost_f), line_ip.c_str());
#else
			strncpy(pcHost_f, line_ip.c_str(), sizeof(pcHost_f) - 1);
			pcHost_f[sizeof(pcHost_f) - 1] = '\0';
#endif
			
			nPort = line_port;

			retval = DoWinsock(pcHost_f, nPort, times_to_ping, ping_interval, include_timestamp, beep_mode, ping_timeout, relookup_interval, auto_exit_on_success, force_send_byte, include_url, use_http, docptr, http_cmd, include_jitter, jitter_sample_size, logfile, use_logfile, ipv, proxy_server, proxy_port, using_credentials, proxy_credentials, only_changes, no_statistics, giveup_count, out, use_source_address, src_address, blocking, always_print_domain, use_color);
			
			int zzz = 0;
			double wakeup = (ping_interval * 1000);
			if (wakeup > 0) {
				while (zzz < wakeup && CTRL_C_ABORT == 0) {
					SLEEP_MS(10); // Use SLEEP_MS
					zzz += 10;
				}
				if (CTRL_C_ABORT == 1) {  // need to be explicit here since breaking the ping loop on an individual host doesn't return in multi mode
					return retval;
				}
			}
		}
		count++;
	}
		return retval;
}
