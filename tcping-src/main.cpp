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

// 当前版本信息
#include "version.h"

// 定义版本字符串常量
const char *TCPING_VERSION = TCPING_VERSION_STR;
const char *TCPING_DATE = TCPING_DATE_STR;

#include "platform.h" // 包含跨平台兼容性头文件

#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <signal.h>

#include "tee.h"
#include "tcping.h"


using namespace std;

void usage(int argc, char* argv[]) {
    cout << "--------------------------------------------------------------" << endl;
    cout << "tcping 跨平台版 - 原始版本由 Eli Fulkerson 开发" << endl;
    cout << "跨平台适配版 - 支持 Windows/Linux/macOS" << endl;
    cout << "--------------------------------------------------------------" << endl;
    cout << endl;
    cout << "用法: " << argv[0] << " [-参数] 服务器地址 [服务器端口]" << endl << endl;
    cout << "完整用法: " << argv[0] << " [-t] [-d] [-i 间隔] [-n 次数] [-w 毫秒] [-b n] [-r 次数] [-s] [-v] [-j] [-js 大小] [-4] [-6] [-c] [-g 次数] [-S 源地址] [--file] [--tee 文件名] [-h] [-u] [--post] [--head] [--proxy-port 端口] [--proxy-server 服务器] [--proxy-credentials 用户名:密码] [-f] 服务器地址 " << "[服务器端口]" << endl << endl;
    cout << " -t     : 持续ping直到按Ctrl-C停止" << endl;
    cout << " -n 5   : 例如，发送5次ping" << endl;
    cout << " -i 5   : 例如，每5秒ping一次" << endl;
    cout << " -w 0.5 : 例如，等待响应0.5秒" << endl;
    cout << " -d     : 在每行输出中包含日期和时间" << endl;
    cout << " -b 1   : 启用蜂鸣声 (1表示连接断开时蜂鸣，2表示连接建立时蜂鸣，" << endl;
    cout << "                     3表示状态变化时蜂鸣，4表示始终蜂鸣)" << endl;
    cout << " -r 5   : 例如，每5次ping重新查询主机名" << endl;
    cout << " -s     : 在ping成功后自动退出"<<endl;                  //[Modification 14 Apr 2011 by Michael Bray, mbray@presidio.com]
    cout << " -v     : 显示版本信息并退出" << endl;
    cout << " -j     : 包含抖动信息，使用默认滚动平均值"<< endl;
	cout << " -js 5  : 包含抖动信息，使用指定大小的滚动平均值，例如5" << endl;
	cout << " --tee  : 将输出镜像到'--tee'后指定的文件中" << endl;
	cout << " --append : 追加到--tee指定的文件，而不是覆盖它" << endl;
	cout << " -4     : 优先使用IPv4" << endl;
	cout << " -6     : 优先使用IPv6" << endl;
	cout << " -c     : 仅在状态变化时显示输出" << endl;
	cout << " --file : 将\"服务器地址\"作为文件名处理，逐行循环处理文件内容" << endl;
	cout << "          注意：--file与-j和-c等选项不兼容，因为它会遍历不同的目标" << endl;
	cout << "          可以选择性地接受服务器端口。例如，\"example.org 443\"是有效的" << endl;
	cout << "          或者，使用-p在命令行中为文件中的所有条目强制指定端口" << endl;
	cout << " -g 5   : 例如，如果连续失败5次则放弃" << endl;
	cout << " -S _X_ : 指定源地址_X_。源必须是客户端计算机的有效IP" << endl;
	cout << " -p _X_ : 指定端口的替代方法" << endl;
	cout << " --fqdn : 在每行中打印域名（如果可用）" << endl;
	cout << " --ansi : 使用ANSI颜色序列（cygwin）" << endl;
	cout << " --color: 使用Windows颜色序列" << endl;
	
    cout << endl << "HTTP选项:" << endl;
    cout << " -h     : HTTP模式（服务器地址使用不带http://的URL）" << endl;
    cout << " -u     : 在每行包含目标URL" << endl;
    cout << " --post : 使用POST而不是GET（可能避免缓存）" << endl;
    cout << " --head : 使用HEAD而不是GET" << endl;
	cout << " --proxy-server : 指定代理服务器" << endl;
	cout << " --proxy-port   : 指定代理端口" << endl;
	cout << " --proxy-credentials : 以username:password格式指定'Proxy-Authorization: Basic'头" << endl;
    cout << endl << "调试选项:" << endl;
    cout << " -f     : 强制tcping至少发送一个字节" << endl;
	cout << " --header : 包含带有原始参数和日期的头。如果使用--tee则隐含此选项" << endl;
	cout << " --block  : 使用'阻塞'套接字连接。这会使-w选项无效，并使用" << endl;
	cout << "            默认超时（在我的情况下长达20秒）。但它可以检测到主动" << endl;
	cout << "            拒绝连接与超时的区别" << endl;
    cout << endl << "\t如果不传递服务器端口，默认为 " << kDefaultServerPort << "。" << endl;

}


int main(int argc, char* argv[]) {

	

    // Do we have enough command line arguments?
    if (argc < 2) {
        usage(argc, argv);
        return 1;
    }

    int times_to_ping = 4;
    int offset = 0;  // because I don't feel like writing a whole command line parsing thing, I just want to accept an optional -t.  // well, that got out of hand quickly didn't it? -Future Eli
    double ping_interval = 1;
    int include_timestamp = 0;
    int beep_mode = 0;  // 0 is off, 1 is down, 2 is up, 3 is on change, 4 is constantly
    int ping_timeout = 2000;
	bool blocking = false;
    int relookup_interval = -1;
    int auto_exit_on_success = 0;
    int force_send_byte = 0;

    int include_url = 0;
    int use_http = 0;
    int http_cmd = 0;

    int include_jitter = 0;
    int jitter_sample_size = 0;

    int only_changes = 0;    // for http mode
    const char *serverptr;
    char *docptr = NULL;
    char server[2048];
    char document[2048];

    // for --tee
    char logfile[256];
    int use_logfile = 0;
	int show_arg_header = 0;
	bool tee_mode_append = false;

    // preferred IP version
    int ipv = 0;

	// http proxy server and port
	int proxy_port = 3128;
	char proxy_server[2048];
	proxy_server[0] = 0;

	char proxy_credentials[2048];
	int using_credentials = 0;

	// Flags for "read from filename" support
	int no_statistics = 0;  // no_statistics flag kills the statistics finale in the cases where we are reading entries from a file
	int reading_from_file = 0;  // setting this flag so we can mangle the other settings against it post parse.  For instance, it moves the meaning of -n and -t
	char urlfile[256];
	int file_times_to_loop = 1;
	bool file_loop_count_was_specific = false;   // ugh, since we are taking over the -n and -t options, but we don't want a default of 4 but we *do* want 4 if they specified 4
	int giveup_count = 0;
	int use_color = 0;

	int use_source_address = 0;
	const char* src_address = "";

	int nPort = kDefaultServerPort;

	int always_print_domain = 0;

	for (int x = 0; x < argc; x++) {

		if (!strcmp(argv[x], "/?") || !strcmp(argv[x], "?") || !strcmp(argv[x], "--help") || !strcmp(argv[x], "-help")) {
			usage(argc, argv);
			return 1;
		}

		if (!strcmp(argv[x], "--proxy-port")) {
			proxy_port = atoi(argv[x + 1]);
			offset = x + 1;
		}
		if (!strcmp(argv[x], "--proxy-server")) {
			#ifdef OS_WINDOWS
			sprintf_s(proxy_server, sizeof(proxy_server), argv[x + 1]);
			#else
			snprintf(proxy_server, sizeof(proxy_server), "%s", argv[x + 1]);
			#endif
			offset = x + 1;
		}

		if (!strcmp(argv[x], "--proxy-credentials")) {
			#ifdef OS_WINDOWS
			sprintf_s(proxy_credentials, sizeof(proxy_credentials), argv[x + 1]);
			#else
			snprintf(proxy_credentials, sizeof(proxy_credentials), "%s", argv[x + 1]);
			#endif
			using_credentials = 1;
			offset = x + 1;
		}

		// force IPv4
		if (!strcmp(argv[x], "-4")) {
			ipv = 4;
			offset = x;
		}

		// force IPv6
		if (!strcmp(argv[x], "-6")) {
			ipv = 6;
			offset = x;
		}

		// ping continuously
		if (!strcmp(argv[x], "-t")) {
			times_to_ping = -1;
			file_loop_count_was_specific = true;
			offset = x;
			cout << endl << "** 持续ping中。按 Control-C 停止 **" << endl;
		}

		// Number of times to ping
		if (!strcmp(argv[x], "-n")) {
			times_to_ping = atoi(argv[x + 1]);
			file_loop_count_was_specific = true;
			offset = x + 1;
		}

		// Give up
		if (!strcmp(argv[x], "-g")) {
			giveup_count = atoi(argv[x + 1]);
			offset = x + 1;
		}

		// exit on first successful ping
		if (!strcmp(argv[x], "-s")) {
			auto_exit_on_success = 1;
			offset = x;
		}

		if (!strcmp(argv[x], "--header")) {
			show_arg_header = 1;
			offset = x;
		}

		if (!strcmp(argv[x], "--block")) {
			blocking = true;
			offset = x;
		}

		if (!strcmp(argv[x], "-p")) {
			nPort = atoi(argv[x + 1]);
			offset = x + 1;
		}

		if (!strcmp(argv[x], "--ansi")) {
			use_color = 1;
			offset = x;
		}

		if (!strcmp(argv[x], "--color")) {
			use_color = 2;
			offset = x;
		}

		if (!strcmp(argv[x], "--fqdn")) {
			always_print_domain = 1;
			offset = x;
		}
		// tee to a log file
		if (!strcmp(argv[x], "--tee")) {
			#ifdef OS_WINDOWS
			strcpy_s(logfile, sizeof(logfile), static_cast<const char*>(argv[x + 1]));
			#else
			strncpy(logfile, argv[x + 1], sizeof(logfile) - 1);
			logfile[sizeof(logfile) - 1] = '\0'; // 确保字符串以null结尾
			#endif
			offset = x + 1;
			use_logfile = 1;
			show_arg_header = 1;
		}

		if (!strcmp(argv[x], "--append")) {
			tee_mode_append = true;
			offset = x;
		}

		// read from a text file
		if (!strcmp(argv[x], "--file")) {
			offset = x;
			no_statistics = 1;
			reading_from_file = 1;
		}

        // http mode
        if (!strcmp(argv[x], "-h")) {
            use_http = 1;
            offset = x;
        }

        // http mode - use get
        if (!strcmp(argv[x], "--get")) {
            use_http = 1; //implied
            http_cmd = HTTP_GET;
            offset = x;
        }

        // http mode - use head
        if (!strcmp(argv[x], "--head")) {
            use_http = 1; //implied
            http_cmd = HTTP_HEAD;
            offset = x;
        }

        // http mode - use post
        if (!strcmp(argv[x], "--post")) {
            use_http = 1; //implied
            http_cmd = HTTP_POST;
            offset = x;
        }

        // include url per line
        if (!strcmp(argv[x], "-u")) {
            include_url = 1;
            offset = x;
        }

        // force send a byte
        if (!strcmp(argv[x], "-f")) {
            force_send_byte = 1;
            offset = x;
        }

        // interval between pings
        if (!strcmp(argv[x], "-i")) {
            ping_interval = atof(argv[x+1]);
            offset = x+1;
        }

        // wait for response
        if (!strcmp(argv[x], "-w")) {
			ping_timeout = (int)(1000 * atof(argv[x + 1]));
            offset = x+1;
        }

		// source address
		if (!strcmp(argv[x], "-S")) {
			src_address = argv[x + 1];
			use_source_address = 1;
			offset = x + 1;
		}

        // optional datetimestamp output
        if (!strcmp(argv[x], "-d")) {
            include_timestamp = 1;
            offset = x;
        }

        // optional jitter output
        if (!strcmp(argv[x], "-j")) {
            include_jitter = 1;
            offset = x;
		}
     
		// optional jitter output (sample size)
		if (!strcmp(argv[x], "-js")) {
            include_jitter = 1;
            offset = x;

            // obnoxious special casing if they actually specify the default 0
            if (!strcmp(argv[x+1], "0")) {
                jitter_sample_size = 0;
                offset = x+1;
            } else {
                if (atoi(argv[x+1]) == 0) {
                    offset = x;
                } else {
                    jitter_sample_size = atoi(argv[x+1]);
                    offset = x+1;
                }
            }
            //			cout << "offset coming out "<< offset << endl;
        }

        // optional hostname re-lookup
        if (!strcmp(argv[x], "-r")) {
            relookup_interval = atoi(argv[x+1]);
            offset = x+1;
        }
		
		 // optional output minimization
        if (!strcmp(argv[x], "-c")) {
            only_changes = 1;
            offset = x;
			cout << endl << "** 仅在状态变化时显示输出。 **" << endl;
        }

        // optional beepage
        if (!strcmp (argv[x], "-b")) {
            beep_mode = atoi(argv[x+1]);
            offset = x+1;        switch (beep_mode) {
            case 0:
                break;
            case 1:
                cout << endl << "** 在连接断开时蜂鸣 - (两声蜂鸣) **" << endl;
                break;
            case 2:
                cout << endl << "** 在连接建立时蜂鸣 - (一声蜂鸣) **" << endl;
                break;
            case 3:
                cout << endl << "** 在状态变化时蜂鸣 - (连接时一声蜂鸣，断开时两声蜂鸣) **" << endl;
                break;
            case 4:
                cout << endl << "** 持续蜂鸣 - (连接时一声蜂鸣，断开时两声蜂鸣) **" << endl;
                break;
            }

        }        // dump version and quit
        if (!strcmp(argv[x], "-v") || !strcmp(argv[x], "--version") ) {
            //cout << "tcping.exe 0.30 Nov 13 2015" << endl;
			cout << "tcping " << TCPING_VERSION << " " << TCPING_DATE << endl;
            cout << "编译日期: " << __DATE__ << " " << __TIME__ <<  endl;
            cout << endl;
            cout << "tcping 原始版本由 Eli Fulkerson 开发" << endl;
            cout << "跨平台适配版支持 Windows/Linux/macOS" << endl;
            cout << endl;
            cout << "-s 选项由 Michael Bray (mbray@presidio.com) 于 2011年4月14日贡献" << endl;
			cout << "包含 base64.cpp - 版权所有 (C) 2004-2008 René Nyffenegger" << endl;
            return 1;
        }
	}

	// open our logfile, if applicable
	tee out;	if (use_logfile == 1) {
		if (tee_mode_append == true) {
			out.OpenAppend(logfile);
		} else {
			out.Open(logfile);
		}
	}



	if (show_arg_header == 1) {
		out.p("-----------------------------------------------------------------\n");
		// print out the args
		out.p("args: ");
		for (int x = 0; x < argc; x++) {
			out.pf("%s ", argv[x]);
		}
		out.p("\n");


		// and the date
		time_t rawtime;
		struct tm  timeinfo;
		char dateStr[32];
		char timeStr[32];

		time(&rawtime);

#ifdef OS_WINDOWS
		errno_t err;
		_strtime_s(timeStr, sizeof(timeStr));
		err = localtime_s(&timeinfo, &rawtime);
#else
		struct tm *tmp = localtime(&rawtime);
		if (tmp) {
			timeinfo = *tmp;
			strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
		}
#endif
		strftime(dateStr, sizeof(dateStr), "%Y:%m:%d", &timeinfo);
		out.pf("date: %s %s\n", dateStr, timeStr);

		// and the attrib
		out.pf("tcping.exe v%s: http://www.elifulkerson.com/projects/tcping.php\n", TCPING_VERSION);
		out.p("-----------------------------------------------------------------\n");

	}




	// Get host and (optionally) port from the command line

	const char* pcHost = "";
	//char pcHost[2048] = "";
	
    if (argc >= 2 + offset) {
		if (!reading_from_file) {
			pcHost = argv[1 + offset];
		}
		else {
#ifdef OS_WINDOWS
			strcpy_s(urlfile, sizeof(urlfile), static_cast<const char*>(argv[offset + 1]));
#else
			strncpy(urlfile, argv[offset + 1], sizeof(urlfile) - 1);
			urlfile[sizeof(urlfile) - 1] = '\0';
#endif
		}


    } else {
			cout << "检查服务器地址前的最后一个参数。您是否指定了一个参数但忘记了它的值？" << endl;
			return 1;
    }

    
	// allow the -p option to win if we set it
    if (argc >= 3 + offset && nPort == kDefaultServerPort) {
        nPort = atoi(argv[2 + offset]);
    }    // 做一些合理性检查
    int nNumArgsIgnored = (argc - 3 - offset);
    if (nNumArgsIgnored > 0) {
        cout << "已忽略 " << nNumArgsIgnored << " 个额外参数。仅供参考。" << endl;
    }    if (use_http == 1 && reading_from_file == 0) {   //added reading from file because if we are doing multiple http this message is just spam.
        // 对于const的pcHost，我们需要创建一个可变的副本
        char pcHostCopy[2048] = {0};
        strncpy(pcHostCopy, pcHost, sizeof(pcHostCopy) - 1);
        
        serverptr = strchr(pcHostCopy, ':');
        if (serverptr != NULL) {
            ++serverptr;
            ++serverptr;
            ++serverptr;
        } else {
            serverptr = pcHostCopy;        }
        const char* docptr_const = strchr(serverptr, '/');
        if (docptr_const != NULL) {
            size_t pos = docptr_const - serverptr;
            pcHostCopy[pos] = '\0';  // Split string at this position
            docptr = pcHostCopy + pos + 1;  // Point docptr after the '/'
            #ifdef OS_WINDOWS
			strcpy_s(server, sizeof(server), static_cast<const char*>(serverptr));
			strcpy_s(document, sizeof(document), static_cast<const char*>(docptr));
            #else
			strncpy(server, serverptr, sizeof(server) - 1);
			server[sizeof(server) - 1] = '\0';			strncpy(document, docptr, sizeof(document) - 1);
			document[sizeof(document) - 1] = '\0';
            #endif
        } else {
            #ifdef OS_WINDOWS
            strcpy_s(server, sizeof(server), static_cast<const char*>(serverptr));
            #else
            strncpy(server, serverptr, sizeof(server) - 1);
            server[sizeof(server) - 1] = '\0';
            #endif
            document[0] = '\0';
        }
		out.pf("\n** 正在从 %s 请求 %s:\n", server, document);
		out.p("(由于各种原因，kbit/s 是一个近似值)\n");
    }    SET_HIGH_PRIORITY();

    // 初始化套接字库
    INIT_SOCKETS();

    // Call the main example routine.
	int retval;

	out.p("\n");

	if (!reading_from_file) {
		retval = DoWinsock_Single(pcHost, nPort, times_to_ping, ping_interval, include_timestamp, beep_mode, ping_timeout, relookup_interval, auto_exit_on_success, force_send_byte, include_url, use_http, docptr, http_cmd, include_jitter, jitter_sample_size, logfile, use_logfile, ipv, proxy_server, proxy_port, using_credentials, proxy_credentials, only_changes, no_statistics, giveup_count, out, use_source_address, src_address, blocking, always_print_domain, use_color);
	}
	else {
		if (file_loop_count_was_specific) {
			file_times_to_loop = times_to_ping;
		}
		times_to_ping = 1;
		retval = DoWinsock_Multi(pcHost, nPort, times_to_ping, ping_interval, include_timestamp, beep_mode, ping_timeout, relookup_interval, auto_exit_on_success, force_send_byte, include_url, use_http, docptr, http_cmd, include_jitter, jitter_sample_size, logfile, use_logfile, ipv, proxy_server, proxy_port, using_credentials, proxy_credentials, only_changes, no_statistics, giveup_count, file_times_to_loop, urlfile, out, use_source_address, src_address, blocking, always_print_domain, use_color);
	}    // Shut Winsock back down and take off.
    CLEANUP_SOCKETS();
    return retval;
}

