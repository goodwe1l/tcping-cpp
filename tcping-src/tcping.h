
#include "tee.h"

extern int DoWinsock_Single(
	const char* pcHost,
	int nPort,
	int times_to_ping,
	double ping_interval,
	int include_timestamp,
	int beep_mode,
	int ping_timeout,
	int relookup_interval,
	int auto_exit_on_success,
	int force_send_byte,
	int include_url,
	int use_http,
	char* docptr,
	int http_cmd,
	int include_jitter,
	int jitter_sample_size,
	char* logfile,
	int use_logfile,
	int ipv,
	char* proxy_server,
	int proxy_port,
	int using_credentials,
	char* proxy_credentials,
	int only_changes,
	int no_statistics,
	int giveup_count,
	class tee &out,
	int use_source_address,
	const char *src_address,
	bool blocking,
	int always_print_domain,
	int use_color
	);

extern int DoWinsock_Multi(
	const char* pcHost,
	int nPort,
	int times_to_ping,
	double ping_interval,
	int include_timestamp,
	int beep_mode,
	int ping_timeout,
	int relookup_interval,
	int auto_exit_on_success,
	int force_send_byte,
	int include_url,
	int use_http,
	char* docptr,
	int http_cmd,
	int include_jitter,
	int jitter_sample_size,
	char* logfile,
	int use_logfile,
	int ipv,
	char* proxy_server,
	int proxy_port,
	int using_credentials,
	char* proxy_credentials,
	int only_changes,
	int no_statistics,
	int giveup_count,
	int file_times_to_loop,
	char* urlfile,
	class tee &out,
	int use_source_address,
	const char *src_address,
	bool blocking,
	int always_print_domain,
	int use_color
	);


const int kDefaultServerPort = 80;
const int HTTP_GET = 0;
const int HTTP_HEAD = 1;
const int HTTP_POST = 2;
