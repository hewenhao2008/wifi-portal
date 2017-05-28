/*
 * Copyright (C) 2017 jianhui zhao <jianhuizhao329@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"
#include "conf.h"

const char *loglevel[] = {"EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG"};

void __debug(const char *filename, int line, int level, const char *format, ...)
{
	struct config *conf = get_config();
	va_list vlist;
	
	if (conf->debuglevel >= level) {
		openlog("wifi-portal", LOG_PID | LOG_CONS, 0);
        va_start(vlist, format);
        vsyslog(level, format, vlist);
        va_end(vlist);
        closelog();
		
		if (conf->debuglevel & LOG_TO_STDERR) {
			char buf[28];
			time_t ts;
			struct tm tm;

			time(&ts);

			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime_r(&ts, &tm));
			
			fprintf(stderr, "[%s][%.24s][%u](%s:%d) ", loglevel[level - LOG_EMERG], buf, getpid(), filename, line);
            va_start(vlist, format);
            vfprintf(stderr, format, vlist);
            va_end(vlist);
            fputc('\n', stderr);
            fflush(stderr);
		}
	}
}

