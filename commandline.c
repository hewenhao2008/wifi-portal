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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "conf.h"
		
static void usage(const char *name)
{
	fprintf(stderr,
		"Usage: %s [OPTION]\n"
		"	-b              Run in background\n"
		"	-d level        Logging to specified levels(0 ~ 7)\n"
		"	-D       	 	Logging to stderr as well\n"
		"	-c file         Configuration file, default is '/etc/wifi-portal.conf'\n"
		"	-p port         Bind to specified port\n"
		"	-s port         Like -p but provide HTTPS on this port\n"
		"	-i interface    Bind to specified interface\n"
		"\n", name
	);
	
	exit(0);
}

void parse_commandline(int argc, char **argv)
{
    int ch;
	struct config *conf = get_config();
	
	while ((ch = getopt(argc, argv, "bd:Dc:p:s:i:")) != -1) {
		switch (ch) {
		case 'b':
			conf->background = true;
			break;

		case 'd':
			conf->debuglevel = atoi(optarg) + LOG_EMERG;
			break;

		case 'D':
			conf->debuglevel |= LOG_TO_STDERR;
			break;
		
		case 'c':
			conf->file = optarg;
			break;
			
		case 'p':
			conf->port = atoi(optarg);
			break;
			
		case 's':
			conf->ssl_port = atoi(optarg);
			break;
			
		case 'i':
			conf->interface = optarg;
			break;
			
		default:
			usage(argv[0]);
		}
	}
	
	if (optind < argc) {
		usage(argv[0]);
	}
	
#if 1	
	printf("background: %d\n", conf->background);
	printf("file: %s\n", conf->file);
	printf("port: %d\n", conf->port);
	printf("ssl_port: %d\n", conf->ssl_port);
	printf("interface: %s\n", conf->interface);
#endif	
}
