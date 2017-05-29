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

static void http_callback_404(struct mg_connection *c)
{
	mg_send_head(c, 404, strlen("about\n"), "Content-Type: text/plain");
	mg_printf(c, "Not Found\n");
}

void http_handle(struct mg_connection *c, struct http_message *hm)
{
	if (mg_vcmp(&hm->uri, "/auth")) {
		mg_send_head(c, 200, strlen("auth\n"), "Content-Type: text/plain");
		mg_printf(c, "auth\n");
	} else if (mg_vcmp(&hm->uri, "about")) {
		mg_send_head(c, 200, strlen("about\n"), "Content-Type: text/plain");
		mg_printf(c, "about\n");
	} else {
		http_callback_404(c);
	}
}

