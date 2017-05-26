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

#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/etherdevice.h>
#include "tmac.h"

#define TMAC_HASH_SIZE 128

rwlock_t tmac_lock;
struct hlist_head tmac_hash_table[TMAC_HASH_SIZE];
static struct kmem_cache *tmac_cache __read_mostly;

static inline u32 tmac_hash_func(u8 *addr)
{
	u32 key = 0;
	/* skip oui */
	key = addr[3] << 8;
	key |= addr[4] << 4;
	key |= addr[5];

	return key & (TMAC_HASH_SIZE - 1);
}

static struct tmac_entry *tmac_alloc(u8 *addr)
{
	struct tmac_entry *tmac = NULL;
	
	tmac = kmem_cache_zalloc(tmac_cache, GFP_ATOMIC);
	if (tmac == NULL) {
		pr_err("tmac_cache: alloc failed\n");
		return NULL;
	}
	
	INIT_HLIST_NODE(&tmac->hlist);
	memcpy(tmac->addr, addr, ETH_ALEN);
	
	return tmac;
}

static inline struct tmac_entry *tmac_find(u8 *addr, struct hlist_head *head)
{
	struct tmac_entry *pos;

	hlist_for_each_entry(pos, head, hlist) {
		if (ether_addr_equal(addr, pos->addr))
			return pos;
	}
	
	return NULL;
}

static void del_tmac(u8 *addr)
{
	u32 hash;
	struct tmac_entry *tmac;
	
	hash = tmac_hash_func(addr);
	
	write_lock_bh(&tmac_lock);
	tmac = tmac_find(addr, &tmac_hash_table[hash]);
	if(tmac) {
		hlist_del(&tmac->hlist);
		kmem_cache_free(tmac_cache, tmac);
	}
	write_unlock_bh(&tmac_lock);
}

static void tmac_clear(void)
{
	int i;
	struct hlist_head *chain;
	struct hlist_node *next;
	struct tmac_entry *pos;
	
	if (!tmac_cache)
		return;
	
	write_lock_bh(&tmac_lock);
	for (i = 0; i != TMAC_HASH_SIZE; i++) {
		chain = &tmac_hash_table[i];
		hlist_for_each_entry_safe(pos, next, chain, hlist) {
			hlist_del(&pos->hlist);
			kmem_cache_free(tmac_cache, pos);
		}

	}
	write_unlock_bh(&tmac_lock);
}

int trusted_mac(u8 *addr)
{
	int ret = 0;
	u32 hash = tmac_hash_func(addr);
	
	read_lock_bh(&tmac_lock);
	if(tmac_find(addr, &tmac_hash_table[hash]))
		ret = 1;
	
	read_unlock_bh(&tmac_lock);
	return ret;
}

int add_tmac(u8 *addr)
{
	u32 hash;
	struct tmac_entry *tmac;
	
	hash = tmac_hash_func(addr);
	
	read_lock_bh(&tmac_lock);
	if(tmac_find(addr, &tmac_hash_table[hash])) {
		read_unlock_bh(&tmac_lock);
	} else {
		read_unlock_bh(&tmac_lock);
		
		tmac = tmac_alloc(addr);
		if (!tmac)
			return -ENOMEM;
		
		write_lock_bh(&tmac_lock);
		hlist_add_head(&tmac->hlist, &tmac_hash_table[hash]);
		write_unlock_bh(&tmac_lock);
	}
	
	return 0;
}

static void *trusted_mac_seq_start(struct seq_file *s, loff_t *pos)
{
	read_lock_bh(&tmac_lock);

	if (*pos == 0)
		return SEQ_START_TOKEN;
	
	if (*pos >= TMAC_HASH_SIZE)
		return NULL;

	return &tmac_hash_table[*pos];
}

static void *trusted_mac_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	if (v == SEQ_START_TOKEN)
		(*pos) = 0;
	else
		(*pos)++;

	if (*pos >= TMAC_HASH_SIZE) {
		return NULL;
	}

	return &tmac_hash_table[*pos];
}

static void trusted_mac_seq_stop(struct seq_file *s, void *v)
{
	read_unlock_bh(&tmac_lock);
}

static int trusted_mac_seq_show(struct seq_file *s, void *v)
{
	struct hlist_head *head = v;
	struct tmac_entry *pos;
	
	if (v == SEQ_START_TOKEN) {
		seq_printf(s, "--------------Trusted MAC Address-------------\n");
	} else {
		hlist_for_each_entry(pos, head, hlist) {
			seq_printf(s, "\t%pM\t\n", &(pos->addr));
		}
	}

	return 0;
}

static struct seq_operations trusted_mac_seq_ops = {
	.start = trusted_mac_seq_start,
	.next = trusted_mac_seq_next,
	.stop = trusted_mac_seq_stop,
	.show = trusted_mac_seq_show
};

static int proc_trusted_mac_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &trusted_mac_seq_ops);
}

static ssize_t proc_trusted_mac_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	char data[128];
	
	if (size == 0)
		return -EINVAL;

	if (size > sizeof(data))
		size = sizeof(data);
	
	if (copy_from_user(data, buf, size))
		return -EFAULT;
	
	data[size - 1] = 0;

	if (!strncmp(data, "clear", 5))
		tmac_clear();
	else {
		char op;
		char addr[ETH_ALEN];
		
		if  (sscanf(data, "%c%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &op, &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]) !=7) {
			pr_err("invalid format: %s\n", data);
			goto QUIT;
		}
		
		op = data[0];
		
		if (op == '+')
			add_tmac(addr);
		else if (op == '-')
			del_tmac(addr);
		else
			pr_err("invalid format: %s\n", data);
	}

QUIT:			
	return size;
}							

const struct file_operations proc_trusted_mac_ops = {
	.owner 		= THIS_MODULE,
	.open  		= proc_trusted_mac_open,
	.read   	= seq_read,
	.write 		= proc_trusted_mac_write,
	.llseek 	= seq_lseek,
	.release 	= seq_release
};

int tmac_init(void)
{
	int i = 0;
	
	tmac_cache = kmem_cache_create("tmac_cache", sizeof(struct tmac_entry), 0, 0, NULL);
	if (!tmac_cache)
		return -ENOMEM;
	
	rwlock_init(&tmac_lock);
	
	for (i = 0; i < TMAC_HASH_SIZE; i++)
		INIT_HLIST_HEAD(&tmac_hash_table[i]);

	return 0;
}

void tmac_free(void)
{
	tmac_clear();
	kmem_cache_destroy(tmac_cache);
}
