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

#ifndef _CONF_H_
#define _CONF_H_

#include <stdbool.h>
#include <syslog.h>

#define LOG_TO_STDERR	(1 << 10)

struct config {
	const char *file;
	bool background;
	const char *interface;
	const char *address;
	const char *id;
	int port;
	int ssl_port;
	int debuglevel;	
};

struct config *get_config();

#endif