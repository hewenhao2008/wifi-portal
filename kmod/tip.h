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
 
#ifndef __TIP_
#define __TIP_

#include <linux/types.h>

struct tip_entry
{
	struct hlist_node	hlist;
	__be32	addr;
};

extern const struct file_operations proc_trusted_ip_ops;

int tip_init(void);
void tip_free(void);
int add_tip(__be32 addr);
int trusted_ip(__be32 addr);

#endif
