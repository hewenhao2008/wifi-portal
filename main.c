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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mongoose.h>

#include "http.h"
#include "debug.h"
#include "utils.h"
#include "conf.h"
#include "commandline.h"




static void ev_handler(struct mg_connection *c, int ev, void *ev_data)
{
	switch (ev) {
	case MG_EV_HTTP_REQUEST:
		http_handle(c, (struct http_message *)ev_data);
		break;
	
	case MG_EV_TIMER:
		check_internet(c->mgr);
		mg_set_timer(c,  mg_time() + 5);
		break;
	default:
		break;
	}
}

int main(int argc, char **argv)
{
	struct config *conf = get_config();
	struct mg_mgr mgr;
	struct mg_connection *nc;
	char address[128] = "";
	
	parse_commandline(argc, argv);
	
	if (conf->background) {
		if (daemon(0, 0) < 0) {
			perror("daemon");
			exit(1);
		}
	}

	if ((conf->address = get_iface_ip(conf->interface)) == NULL) {
		debug(LOG_ERR, "Could not get IP address information of %s, exiting...", conf->interface);
		exit(1);
	}

	debug(LOG_DEBUG, "Found IP address of %s: %s", conf->interface, conf->address);
	
	if ((conf->id = get_iface_mac(conf->interface)) == NULL) {
		debug(LOG_ERR, "Could not get MAC address information of %s, exiting...", conf->interface);
		exit(1);
	}

	debug(LOG_DEBUG, "Found MAC address of %s: %s", conf->interface, conf->id);

	snprintf(address, sizeof(address), "%s:%d", conf->address, conf->port);
	
	mg_mgr_init(&mgr, NULL);
	
	nc = mg_bind(&mgr, address, ev_handler);
	mg_set_protocol_http_websocket(nc);

	mg_set_timer(nc, mg_time());

	for (;;) {
		mg_mgr_poll(&mgr, 1000);
	}
	mg_mgr_free(&mgr);

	return 0;
}
