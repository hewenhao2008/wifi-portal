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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/inetdevice.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_l3proto.h>

#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "tip.h"
#include "tmac.h"

static struct proc_dir_entry *proc;
static char gw_interface[32] = "br-lan";
static int gw_interface_ifindex = -1;
static __be32 gw_interface_ipaddr;
static int gw_port = 2060;
static int gw_ssl_port = 8443;
static int wifidog_enabled;

static int update_gw_interface(const char *interface)
{
	int ret = 0;
	struct net_device *dev;
	struct in_device *in_dev;
	
	dev = dev_get_by_name(&init_net, interface);
	if (!dev) {
		pr_err("Not found interface: %s\n", interface);
		return -ENOENT;
	}
	
	gw_interface_ifindex = dev->ifindex;
	
	in_dev = inetdev_by_index(dev_net(dev), gw_interface_ifindex);
	if (!in_dev) {
		pr_err("Not found in_dev on %s\n", interface);
		ret = -ENOENT;
		goto QUIT;
	}
	
	for_primary_ifa(in_dev) {
		gw_interface_ipaddr = ifa->ifa_local;
		pr_info("Found ip from %s: %pI4\n", interface, &gw_interface_ipaddr);
		break;
	} endfor_ifa(in_dev)
	
QUIT:	
	dev_put(dev);
	
	return ret;
}

static int proc_config_show(struct seq_file *s, void *v)
{
	seq_printf(s, "enabled = %d\n", wifidog_enabled);
	seq_printf(s, "interface = %s\n", gw_interface);
	seq_printf(s, "port = %d\n", gw_port);
	seq_printf(s, "ssl_port = %d\n", gw_ssl_port);
	
	return 0;
}

static ssize_t proc_config_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	char data[128];
	char *delim, *key;
	const char *value;
	
	if (size == 0)
		return -EINVAL;

	if (size > sizeof(data))
		size = sizeof(data);
	
	if (copy_from_user(data, buf, size))
		return -EFAULT;
	
	data[size - 1] = 0;
	
	key = data;
	while (key && *key) {
		while (*key && (*key == ' '))
			key++;
		
		delim = strchr(key, '=');
		if (!delim)
			break;
		
		*delim++ = 0;
		value = delim;
		
		delim = strchr(value, ' ');
		if (delim)
			*delim++ = 0;
		
		if (!strcmp(key, "enabled"))
			wifidog_enabled = simple_strtol(value, NULL, 0);
		else if (!strcmp(key, "interface")) {
			strncpy(gw_interface, value, sizeof(gw_interface));
			update_gw_interface(gw_interface);
		} else if (!strcmp(key, "port"))
			gw_port = simple_strtol(value, NULL, 0);
		else if (!strcmp(key, "ssl_port"))
			gw_ssl_port = simple_strtol(value, NULL, 0);
		
		key = delim;
	}
			
	return size;
}

static int proc_config_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_config_show, NULL);
}

const static struct file_operations proc_config_ops = {
	.owner 		= THIS_MODULE,
	.open  		= proc_config_open,
	.read   	= seq_read,
	.write		= proc_config_write,
	.llseek 	= seq_lseek,
	.release 	= single_release
};

static u32 __nf_nat_setup_info(void *priv, struct sk_buff *skb, const struct nf_hook_state *state, struct nf_conn *ct)
{
	struct tcphdr *tph = tcp_hdr(skb);
	union nf_conntrack_man_proto proto;
	struct nf_nat_range newrange;
	
	if (tph->dest == htons(80))
		proto.tcp.port = htons(gw_port);
	else
		proto.tcp.port = htons(gw_ssl_port);

	newrange.flags	     = NF_NAT_RANGE_MAP_IPS | NF_NAT_RANGE_PROTO_SPECIFIED;
	newrange.min_addr.ip = newrange.max_addr.ip = gw_interface_ipaddr;
	newrange.min_proto   = newrange.max_proto = proto;

	return nf_nat_setup_info(ct, &newrange, NF_NAT_MANIP_DST);
}

static u32 wifidog_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct ethhdr *ehdr;
	struct iphdr *iph;
	u8 protocol;
	
	if (unlikely(!wifidog_enabled))
		return NF_ACCEPT;
	
	if (unlikely(gw_interface_ifindex < 0) ||  state->in->ifindex != gw_interface_ifindex)
		return NF_ACCEPT;
	
	ehdr = eth_hdr(skb);
	iph = ip_hdr(skb);
	
	if (iph->daddr == gw_interface_ipaddr)
		return NF_ACCEPT;
	
	if (trusted_ip(iph->daddr))
		return NF_ACCEPT;
	
	if (trusted_mac(ehdr->h_source))
		return NF_ACCEPT;
	
	protocol = iph->protocol;
	
	if (protocol == IPPROTO_UDP) {
		struct udphdr *uph = udp_hdr(skb);
		if (uph->dest == htons(53) || uph->dest == htons(67))
			return NF_ACCEPT;
		return NF_DROP;
	} else if (protocol == IPPROTO_TCP) {
		struct tcphdr *tph = tcp_hdr(skb);
		if (tph->dest == htons(80) || tph->dest == htons(443))
			return nf_nat_ipv4_in(priv, skb, state, __nf_nat_setup_info);
		return NF_DROP;
	}
	
	return NF_ACCEPT;
}

static struct nf_hook_ops wifidog_ops __read_mostly = {
	.hook		= wifidog_hook,
	.pf			= PF_INET,
	.hooknum	= NF_INET_PRE_ROUTING,
	.priority	= NF_IP_PRI_CONNTRACK + 1 /* after conntrack */
};

static int __init wifidog_init(void)
{
	int ret;
	
	update_gw_interface(gw_interface);
	
	proc = proc_mkdir("wifidog", NULL);
	if (!proc) {
		pr_err("can't create dir /proc/wifidog/\n");
		return -ENODEV;;
	}
	
	if (!proc_create("config", 0644, proc, &proc_config_ops)) {
		pr_err("can't create file /proc/wifidog/config\n");
		ret = -EINVAL;
		goto remove;
	}
	
	if (!proc_create("trusted_ip", 0644, proc, &proc_trusted_ip_ops)) {
		pr_err("can't create file /proc/wifidog/trusted_ip\n");
		ret = -EINVAL;
		goto remove_config;
	}
	
	if (!proc_create("trusted_mac", 0644, proc, &proc_trusted_mac_ops)) {
		pr_err("can't create file /proc/wifidog/trusted_mac\n");
		ret = -EINVAL;
		goto remove_trusted_ip;
	}
	
	ret = tip_init();
	if (ret) {
		pr_err("tip_init failed\n");
		goto remove_trusted_mac;
	}
	
	ret = tmac_init();
	if (ret) {
		pr_err("tmac_init failed\n");
		goto free_tip;
	}
	
	ret = nf_register_hook(&wifidog_ops);
	if (ret < 0) {
		pr_err("can't register hook\n");
		goto free_tmac;
	}

	pr_info("kmod of wifidog is started\n");

	return 0;

free_tmac:	
	tmac_free();
free_tip:
	tip_free();
remove_trusted_mac:
	remove_proc_entry("trusted_mac", proc);
remove_trusted_ip:	
	remove_proc_entry("trusted_ip", proc);
remove_config:	
	remove_proc_entry("config", proc);
remove:
	remove_proc_entry("wifidog", NULL);
	return ret;
}

static void __exit wifidog_exit(void)
{
	remove_proc_entry("trusted_mac", proc);
	remove_proc_entry("trusted_ip", proc);
	remove_proc_entry("config", proc);
	remove_proc_entry("wifidog", NULL);
	nf_unregister_hook(&wifidog_ops);
	
	tip_free();
	tmac_free();
	
	pr_info("kmod of wifidog is stop\n");
}

module_init(wifidog_init);
module_exit(wifidog_exit);

MODULE_AUTHOR("jianhui zhao <jianhuizhao329@gmail.com>");
MODULE_LICENSE("GPL");
