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

#include "http.h"

static const struct mg_str api_about = MG_MK_STR("/about");
static const struct mg_str api_auth = MG_MK_STR("/auth");

static int uri_equal(struct mg_str *uri, const struct mg_str *prefix)
{
	return uri->len == prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static void http_callback_404(struct mg_connection *c)
{
	printf("x\n");
}

void http_handle(struct mg_connection *c, struct http_message *hm)
{
	if (uri_equal(&hm->uri, &api_auth)) {
		mg_send_head(c, 200, strlen("auth\n"), "Content-Type: text/plain");
		mg_printf(c, "auth\n");
	} else if (uri_equal(&hm->uri, &api_about)) {
		mg_send_head(c, 200, strlen("about\n"), "Content-Type: text/plain");
		mg_printf(c, "about\n");
	} else
		http_callback_404(c);
}

static void ev_check_internet_handler(struct mg_connection *c, int ev, void *ev_data)
{
	switch (ev) {
	case MG_EV_CONNECT:
		if (*(int *) ev_data != 0) {
			fprintf(stderr, "connect() failed: %s\n", strerror(*(int *) ev_data));
		}
	case MG_EV_HTTP_REPLY:
		c->flags |= MG_F_CLOSE_IMMEDIATELY;
		printf("reply\n");
	default:
		break;
	}
}

void check_internet(struct mg_mgr *mgr)
{
	mg_connect_http(mgr, ev_check_internet_handler, "http://m.baidu.com/favicon.ico", NULL, NULL);
}

