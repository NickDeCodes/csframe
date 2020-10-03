/*************************************************************************
	> File Name: log.h
	> Author: 
	> Mail: 
	> Created Time: 三  8/19 21:43:20 2020
 ************************************************************************/

#ifndef _LOG_H
#define _LOG_H

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

/* 支持系统日志模式 */
typedef enum {
	SYSLOG_FACILITY_DAEMON, // 后台进程相关信息
	SYSLOG_FACILITY_USER, // 用户进程信息
	SYSLOG_FACILITY_AUTH, // 特权信息
#ifdef LOG_AUTHPRIV
	SYSLOG_FACILITY_AUTHPRIV, // 包括特权信息如用户名在内的认证活动
#endif
	SYSLOG_FACILITY_LOCAL0, // 本地用户信息0
	SYSLOG_FACILITY_LOCAL1, // 本地用户信息1
	SYSLOG_FACILITY_LOCAL2, // 本地用户信息2
	SYSLOG_FACILITY_LOCAL3, // 本地用户信息3
	SYSLOG_FACILITY_LOCAL4, // 本地用户信息4
	SYSLOG_FACILITY_LOCAL5, // 本地用户信息5
	SYSLOG_FACILITY_LOCAL6, // 本地用户信息6
	SYSLOG_FACILITY_LOCAL7, // 本地用户信息7
	SYSLOG_FACILITY_NOT_SET = -1
} SyslogFacility;

/* 日志水平 */
typedef enum {
	SYSLOG_LEVEL_QUIET, // 安静输出模式
	SYSLOG_LEVEL_FATAL, // 致命错误模式
	SYSLOG_LEVEL_ERROR, // 错误警告模式
	SYSLOG_LEVEL_INFO, // 重要信息模式
	SYSLOG_LEVEL_VERBOSE, // 完整信息模式
	SYSLOG_LEVEL_DEBUG1, // 调试信息模式1
	SYSLOG_LEVEL_DEBUG2, // 调试信息模式2
	SYSLOG_LEVEL_DEBUG3, // 调试信息模式3
	SYSLOG_LEVEL_NOT_SET = -1
} LogLevel;

static struct {
	const char *name;
	SyslogFacility val;
} log_facilities[] = {
	{ "DAEMON",	SYSLOG_FACILITY_DAEMON },
	{ "USER",	SYSLOG_FACILITY_USER },
	{ "AUTH",	SYSLOG_FACILITY_AUTH },
#ifdef LOG_AUTHPRIV
	{ "AUTHPRIV",	SYSLOG_FACILITY_AUTHPRIV },
#endif
	{ "LOCAL0",	SYSLOG_FACILITY_LOCAL0 },
	{ "LOCAL1",	SYSLOG_FACILITY_LOCAL1 },
	{ "LOCAL2",	SYSLOG_FACILITY_LOCAL2 },
	{ "LOCAL3",	SYSLOG_FACILITY_LOCAL3 },
	{ "LOCAL4",	SYSLOG_FACILITY_LOCAL4 },
	{ "LOCAL5",	SYSLOG_FACILITY_LOCAL5 },
	{ "LOCAL6",	SYSLOG_FACILITY_LOCAL6 },
	{ "LOCAL7",	SYSLOG_FACILITY_LOCAL7 },
	{ NULL,		SYSLOG_FACILITY_NOT_SET }
};

static struct {
	const char *name;
	LogLevel val;
} log_levels[] = {
	{ "QUIET",	SYSLOG_LEVEL_QUIET },
	{ "FATAL",	SYSLOG_LEVEL_FATAL },
	{ "ERROR",	SYSLOG_LEVEL_ERROR },
	{ "INFO",	SYSLOG_LEVEL_INFO },
	{ "VERBOSE",	SYSLOG_LEVEL_VERBOSE },
	{ "DEBUG",	SYSLOG_LEVEL_DEBUG1 },
	{ "DEBUG1",	SYSLOG_LEVEL_DEBUG1 },
	{ "DEBUG2",	SYSLOG_LEVEL_DEBUG2 },
	{ "DEBUG3",	SYSLOG_LEVEL_DEBUG3 },
	{ NULL,		SYSLOG_LEVEL_NOT_SET }
};

#define MSGBUFSIZE 1024
typedef void (log_handler_fn)(LogLevel, const char *, void *);

static LogLevel log_level = SYSLOG_LEVEL_INFO;
static log_handler_fn *log_handler;
static int log_on_stderr = 1;
static int log_stderr_fd = STDERR_FILENO;
static int log_facility = LOG_AUTH;
static char *progname;
static void *log_handler_ctx;

/* 初始化日志 */
void log_init(char *name, LogLevel level, SyslogFacility facility, int on_stderr);
/* 检测日志水平是否合法 */
int log_change_level(LogLevel new_log_level);
/* 执行日志 */
void do_log(LogLevel level, const char *fmt, va_list args);
/* 获取当前日志水平 */
LogLevel log_level_get(void);
/* 获取当前日志工厂 */
SyslogFacility log_facility_number(char *name);
/* 获取当前日志工厂名字 */
const char *log_facility_name(SyslogFacility facility);
/* 获取当前日志工厂编号 */
LogLevel log_level_number(char *name);
/* 获取当前日志水平名字 */
const char *log_level_name(LogLevel level);
/* 记录错误日志 */
void error(const char *fmt, ...);
/* 日志结束 */
void logdie(const char *fmt, ...);
/* Log this message (information that usually should go to the log). */
void logit(const char *fmt, ...);
/* More detailed messages (information that does not need to go to the log). */
void verbose(const char *fmt, ...);
/* Debugging messages that should not be logged during normal operation. */
void debug(const char *fmt, ...);
void debug2(const char *fmt, ...);
void debug3(const char *fmt, ...);
int log_is_on_stderr(void);
/* redirect what would usually get written to stderr to specified file */
void log_redirect_stderr_to(const char *logfile);
void set_log_handler(log_handler_fn *handler, void *ctx);
void do_log2(LogLevel level, const char *fmt, ...);

#endif
