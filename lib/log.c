/*************************************************************************
	> File Name: log.c
	> Author: 
	> Mail: 
	> Created Time: 三  8/19 21:43:59 2020
 ************************************************************************/

#include "log.h"

/* 初始化日志 */
void log_init(char *name, LogLevel level, SyslogFacility facility, int on_stderr) {
	progname = name;

	if (log_change_level(level) != 0) {
		fprintf(stderr, "Unrecognized internal syslog level code %d\n", (int)level);
		exit(1);
	}

	log_handler = NULL;
	log_handler_ctx = NULL;

	log_on_stderr = on_stderr;
	if (on_stderr)
		return;

	switch (facility) {
	case SYSLOG_FACILITY_DAEMON:
		log_facility = LOG_DAEMON;
		break;
	case SYSLOG_FACILITY_USER:
		log_facility = LOG_USER;
		break;
	case SYSLOG_FACILITY_AUTH:
		log_facility = LOG_AUTH;
		break;
#ifdef LOG_AUTHPRIV
	case SYSLOG_FACILITY_AUTHPRIV:
		log_facility = LOG_AUTHPRIV;
		break;
#endif
	case SYSLOG_FACILITY_LOCAL0:
		log_facility = LOG_LOCAL0;
		break;
	case SYSLOG_FACILITY_LOCAL1:
		log_facility = LOG_LOCAL1;
		break;
	case SYSLOG_FACILITY_LOCAL2:
		log_facility = LOG_LOCAL2;
		break;
	case SYSLOG_FACILITY_LOCAL3:
		log_facility = LOG_LOCAL3;
		break;
	case SYSLOG_FACILITY_LOCAL4:
		log_facility = LOG_LOCAL4;
		break;
	case SYSLOG_FACILITY_LOCAL5:
		log_facility = LOG_LOCAL5;
		break;
	case SYSLOG_FACILITY_LOCAL6:
		log_facility = LOG_LOCAL6;
		break;
	case SYSLOG_FACILITY_LOCAL7:
		log_facility = LOG_LOCAL7;
		break;
	default:
		fprintf(stderr, "Unrecognized internal syslog facility code %d\n", (int)facility);
		exit(1);
	}

	openlog(progname ? progname : "unkown", LOG_PID, log_facility);
	closelog();
}

/* 检测日志水平是否合法 */
int log_change_level(LogLevel new_log_level) {
	if (progname == NULL)
		return 0;

	switch (new_log_level) {
	case SYSLOG_LEVEL_QUIET:
	case SYSLOG_LEVEL_FATAL:
	case SYSLOG_LEVEL_ERROR:
	case SYSLOG_LEVEL_INFO:
	case SYSLOG_LEVEL_VERBOSE:
	case SYSLOG_LEVEL_DEBUG1:
	case SYSLOG_LEVEL_DEBUG2:
	case SYSLOG_LEVEL_DEBUG3:
		log_level = new_log_level;
		return 0;
	default:
		return -1;
	}
}

/* 执行日志 */
void do_log(LogLevel level, const char *fmt, va_list args) {
	char msgbuf[MSGBUFSIZE];
	char fmtbuf[MSGBUFSIZE];
	char *txt = NULL;
	int pri = LOG_INFO;
	int saved_errno = errno;
	log_handler_fn *tmp_handler;

	if (level > log_level)
		return;

	switch (level) {
	case SYSLOG_LEVEL_FATAL:
		if (!log_on_stderr)
			txt = "fatal";
		pri = LOG_CRIT; // 重要情况
		break;
	case SYSLOG_LEVEL_ERROR:
		if (!log_on_stderr)
			txt = "error";
		pri = LOG_ERR; // 错误
		break;
	case SYSLOG_LEVEL_INFO:
		pri = LOG_INFO; // 情报信息
		break;
	case SYSLOG_LEVEL_VERBOSE:
		pri = LOG_INFO; // 情报信息
		break;
	case SYSLOG_LEVEL_DEBUG1:
		txt = "debug1";
		pri = LOG_DEBUG; // 调试信息1
		break;
	case SYSLOG_LEVEL_DEBUG2:
		txt = "debug2";
		pri = LOG_DEBUG; // 调试信息2
		break;
	case SYSLOG_LEVEL_DEBUG3:
		txt = "debug3";
		pri = LOG_DEBUG; // 调试信息3
		break;
	default:
		txt = "internal error";
		pri = LOG_ERR; // 未支持的水平
		break;
	}
	if (txt != NULL && log_handler == NULL) {
		snprintf(fmtbuf, sizeof(fmtbuf), "%s: %s", txt, fmt);
		vsnprintf(msgbuf, sizeof(msgbuf), fmtbuf, args);
	} else {
		vsnprintf(msgbuf, sizeof(msgbuf), fmt, args);
	}
	if (log_handler != NULL) {
		/* Avoid recursion */
		tmp_handler = log_handler;
		log_handler = NULL;
		tmp_handler(level, fmtbuf, log_handler_ctx);
		log_handler = tmp_handler;
	} else if (log_on_stderr) {
		snprintf(msgbuf, sizeof msgbuf, "%.*s\r\n", (int)sizeof msgbuf - 3, fmtbuf);
		(void)write(log_stderr_fd, msgbuf, strlen(msgbuf));
	} else {
		openlog(progname ? progname : "unkown", LOG_PID, log_facility);
		syslog(pri, "%.500s", fmtbuf);
		closelog();
	}
	errno = saved_errno;
}

/* 获取当前日志水平 */
LogLevel log_level_get(void) {
	return log_level;
}

/* 获取当前日志工厂 */
SyslogFacility log_facility_number(char *name) {
	int i;

	if (name != NULL)
		for (i = 0; log_facilities[i].name; i++)
			if (strcasecmp(log_facilities[i].name, name) == 0)
				return log_facilities[i].val;
	return SYSLOG_FACILITY_NOT_SET;
}

/* 获取当前日志工厂名字 */
const char *log_facility_name(SyslogFacility facility) {
	int i;

	for (i = 0; log_facilities[i].name; i++)
		if (log_facilities[i].val == facility)
			return log_facilities[i].name;
	return NULL;
}

/* 获取当前日志工厂编号 */
LogLevel log_level_number(char *name) {
	int i;

	if (name != NULL)
		for (i = 0; log_levels[i].name; i++)
			if (strcasecmp(log_levels[i].name, name) == 0)
				return log_levels[i].val;
	return SYSLOG_LEVEL_NOT_SET;
}

/* 获取当前日志水平名字 */
const char *log_level_name(LogLevel level) {
	int i;

	for (i = 0; log_levels[i].name != NULL; i++)
		if (log_levels[i].val == level)
			return log_levels[i].name;
	return NULL;
}

/* 记录错误日志 */
void error(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	do_log(SYSLOG_LEVEL_ERROR, fmt, args);
	va_end(args);
}

/* 日志结束 */
void logdie(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	do_log(SYSLOG_LEVEL_INFO, fmt, args);
	va_end(args);
	exit(255);
}

/* Log this message (information that usually should go to the log). */
void logit(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	do_log(SYSLOG_LEVEL_INFO, fmt, args);
	va_end(args);
}

/* More detailed messages (information that does not need to go to the log). */
void verbose(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	do_log(SYSLOG_LEVEL_VERBOSE, fmt, args);
	va_end(args);
}

/* Debugging messages that should not be logged during normal operation. */
void debug(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	do_log(SYSLOG_LEVEL_DEBUG1, fmt, args);
	va_end(args);
}

void debug2(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	do_log(SYSLOG_LEVEL_DEBUG2, fmt, args);
	va_end(args);
}

void debug3(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	do_log(SYSLOG_LEVEL_DEBUG3, fmt, args);
	va_end(args);
}

int log_is_on_stderr(void) {
	return log_on_stderr && log_stderr_fd == STDERR_FILENO;
}

/* redirect what would usually get written to stderr to specified file */
void log_redirect_stderr_to(const char *logfile) {
	int fd;

	if ((fd = open(logfile, O_WRONLY|O_CREAT|O_APPEND, 0600)) == -1) {
		fprintf(stderr, "Couldn't open logfile %s: %s\n", logfile, strerror(errno));
		exit(1);
	}
	log_stderr_fd = fd;
}

void set_log_handler(log_handler_fn *handler, void *ctx) {
	log_handler = handler;
	log_handler_ctx = ctx;
}

void do_log2(LogLevel level, const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	do_log(level, fmt, args);
	va_end(args);
}

