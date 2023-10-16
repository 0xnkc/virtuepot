/*
 * Copyright (c) 2003 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <sys/types.h>
#include <sys/param.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/ioctl.h>
#include <sys/tree.h>
#include <sys/queue.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_NET_BPF_H
#include <net/bpf.h>
#endif

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <event.h>
#include <pcap.h>
#include <dnet.h>

#include "honeyd.h"
#include "interface.h"
#include "network.h"
#include "router.h"			/* for network compare */
#include "debug.h"

/* Prototypes */
int pcap_dloff(pcap_t *);

void honeyd_recv_cb(u_char *, const struct pcap_pkthdr *, const u_char *);

static char *interface_expandips(int, char **, int);
static void interface_recv(int, short, void *);
static void interface_poll_recv(int, short, void *);

int interface_verify_config = 0;
int interface_dopoll;
char *interface_filter = NULL;

static TAILQ_HEAD(ifq, interface) interfaces;
static intf_t *intf;
static pcap_handler if_recv_cb = NULL;

void
interface_prevent_init(void)
{
#ifdef DISABLE_KQUEUE
	if (!interface_dopoll) 
		setenv("EVENT_NOKQUEUE", "yes", 1);
#endif
#ifdef DISABLE_POLL
	if (!interface_dopoll) 
		setenv("EVENT_NOPOLL", "yes", 1);
#endif
}

// Set the callback function to be called when external packet are recieved via pcap
void
interface_initialize(pcap_handler cb)
{
	TAILQ_INIT(&interfaces);

	if ((intf = intf_open()) == NULL)
	{
		syslog(LOG_ERR, "%s: intf_open",__func__);
		exit(EXIT_FAILURE);
	}

	if_recv_cb = cb;
}

/* Get a new interface structure */

static struct interface *
interface_new(char *dev)
{
	char ebuf[PCAP_ERRBUF_SIZE];
	struct interface *inter;

	if ((inter = calloc(1, sizeof(struct interface))) == NULL)
	{
		syslog(LOG_ERR, "%s: calloc", __func__);
		exit(EXIT_FAILURE);
	}

	if (dev == NULL) {
		if ((dev = pcap_lookupdev(ebuf)) == NULL)
		{
			syslog(LOG_ERR, "pcap_lookupdev: %s",ebuf);
			exit(EXIT_FAILURE);
		}
	}

	TAILQ_INSERT_TAIL(&interfaces, inter, next);

	inter->if_ent.intf_len = sizeof(struct intf_entry);
	strlcpy(inter->if_ent.intf_name, dev, sizeof(inter->if_ent.intf_name));
	
	if (intf_get(intf, &inter->if_ent) < 0)
	{
		syslog(LOG_ERR, "%s: intf_get", __func__);
		exit(EXIT_FAILURE);
	}

	if (inter->if_ent.intf_addr.addr_type != ADDR_TYPE_IP)
		warn("%s: bad interface configuration: %s is not IP",
		    __func__, dev);

	return (inter);
}

/*
 * Returns the number of configured interfaces
 */

int
interface_count(void)
{
	struct interface *inter;
	int count = 0;

	TAILQ_FOREACH(inter, &interfaces, next)
		count++;

	return (count);
}

/*
 * Returns the interface with the specified offset in the list
 */

struct interface *
interface_get(int off)
{
	struct interface *inter;
	int count = 0;

	TAILQ_FOREACH(inter, &interfaces, next) {
		if (count++ == off)
			return inter;
	}

	return (NULL);
}

struct interface *
interface_find(char *name)
{
	struct interface *inter;

	TAILQ_FOREACH(inter, &interfaces, next) {
		if (strcasecmp(inter->if_ent.intf_name, name) == 0)
			return (inter);
	}

	return (NULL);
}

struct interface *
interface_find_addr(struct addr *addr)
{
	struct interface *inter;

	TAILQ_FOREACH(inter, &interfaces, next) {
		if (addr_cmp(addr, &inter->if_ent.intf_addr) == 0)
			return (inter);
	}

	return (NULL);
}

struct interface *
interface_find_responsible(struct addr *addr)
{
	struct interface *inter;
	struct network net, ifnet;

	net.net = *addr;

	TAILQ_FOREACH(inter, &interfaces, next) {
		/* 
		 * Restore the original address so that the network
		 * comparison gets the correct network bits.
		 */
		ifnet.net = inter->if_ent.intf_addr;
		ifnet.net.addr_bits = inter->if_addrbits;
		addr_net(&ifnet.net, &ifnet.net);
		ifnet.net.addr_bits = inter->if_addrbits;
		if (network_compare(&ifnet, &net) == NET_CONTAINS)
			return (inter);
	}

	return (NULL);
}

void
interface_close(struct interface *inter)
{
	TAILQ_REMOVE(&interfaces, inter, next);

	if (inter->if_eth != NULL)
		eth_close(inter->if_eth);
	pcap_close(inter->if_pcap);

	free(inter);
}

void
interface_close_all(void)
{
	struct interface *inter;

	while((inter = TAILQ_FIRST(&interfaces)) != NULL)
		interface_close(inter);
}

void
interface_ether_filter(struct interface *inter,
    int naddresses, char **addresses)
{
	char line[48];
	char *dst;

	dst = interface_expandips(naddresses, addresses, 0);

	if (snprintf(inter->if_filter, sizeof(inter->if_filter),
		"(arp or ip proto 47 or "
		"(udp and src port 67 and dst port 68) or (ip %s%s%s))",
		dst ? "and (" : "", dst ? dst : "", dst ? ")" : "") >= 
	    sizeof(inter->if_filter))
	{
		syslog(LOG_ERR, "%s: pcap filter exceeds maximum length", __func__);
		exit(EXIT_FAILURE);
	}

	inter->if_eth = eth_open(inter->if_ent.intf_name);
	if (inter->if_eth == NULL)
	{
		syslog(LOG_ERR, "%s: eth_open: %s",__func__, inter->if_ent.intf_name);
		exit(EXIT_FAILURE);
	}

	snprintf(line, sizeof(line), " and not ether src %s",
	    addr_ntoa(&inter->if_ent.intf_link_addr));
	strlcat(inter->if_filter, line, sizeof(inter->if_filter));
}

void
interface_regular_filter(struct interface *inter,
    int naddresses, char **addresses)
{
	char *dst;

	/* Destination addresses only */
	dst = interface_expandips(naddresses, addresses, 1);

	if (snprintf(inter->if_filter, sizeof(inter->if_filter),
		"ip %s%s%s",
		dst ? "and (" : "", dst ? dst : "", dst ? ")" : "") >= 
	    sizeof(inter->if_filter))
	{
		syslog(LOG_ERR, "%s: pcap filter exceeds maximum length",__func__);
		exit(EXIT_FAILURE);
	}
}

void
interface_init(char *dev, int naddresses, char **addresses)
{
	int i;
	struct bpf_program fcode;
	char ebuf[PCAP_ERRBUF_SIZE];
	struct interface *inter;
	int time, promisc = 0;
	int pcap_fd;

	if (dev != NULL && interface_find(dev) != NULL) {
		fprintf(stderr, "Warning: Interface %s already configured\n",
		    dev);
		return;
	}

	inter = interface_new(dev);

	if (interface_filter == NULL) {
		/* 
		 * Compute the monitored IP addresses.  If we are ethernet,
		 * ignore our own packets.
		 */
		if (inter->if_ent.intf_link_addr.addr_type == ADDR_TYPE_ETH) {
			interface_ether_filter(inter, naddresses, addresses);

			/* 
			 * We open all interfaces before parsing the
			 * configuration, this means that for now, we
			 * open all ethernet interfaces in promiscuous
			 * mode.
			 */

			promisc = 1;
		} else {
			interface_regular_filter(inter, naddresses, addresses);
		}
	} else {
		promisc = 1;

		/* Use an externally supplied filter */
		strlcpy(inter->if_filter, interface_filter,
		    sizeof(inter->if_filter));
	}

	/* In most cases, we want to compare the addresses directly */
	inter->if_addrbits = inter->if_ent.intf_addr.addr_bits;
	inter->if_ent.intf_addr.addr_bits = IP_ADDR_BITS;
	

	inter->subnetBcastAddress = ntohl((uint32_t)inter->if_ent.intf_addr.__addr_u.__ip);
	for (i = 0; i < 32 - inter->if_addrbits; i++)
		inter->subnetBcastAddress |= (0 | (1 << i));
	inter->subnetBcastAddress = htonl(inter->subnetBcastAddress);

	/* Don't open interfaces for real if we just want to verify config */
	if (interface_verify_config)
		return;

	time = interface_dopoll ? 10 : 30;
	if ((inter->if_pcap = pcap_open_live(inter->if_ent.intf_name,
		 inter->if_ent.intf_mtu + 40, promisc, time, ebuf)) == NULL)
	{
		syslog(LOG_ERR, "pcap_open_live: %s",ebuf);
		exit(EXIT_FAILURE);
	}

	/* Get offset to packet data */
	inter->if_dloff = pcap_dloff(inter->if_pcap);
	
	syslog(LOG_INFO, "listening %son %s: %s",
	    promisc ? "promiscuously " : "",
	    inter->if_ent.intf_name, inter->if_filter);

	if (pcap_compile(inter->if_pcap, &fcode, inter->if_filter, 1, 0) < 0 ||
	    pcap_setfilter(inter->if_pcap, &fcode) < 0)
	{
		syslog(LOG_ERR, "bad pcap filter: %s", pcap_geterr(inter->if_pcap));
		exit(EXIT_FAILURE);
	}

#ifdef HAVE_PCAP_GET_SELECTABLE_FD
	pcap_fd = pcap_get_selectable_fd(inter->if_pcap);
#else
	pcap_fd = pcap_fileno(inter->if_pcap);
#endif
#if defined(BIOCIMMEDIATE)
	{
		int on = 1;
		DFPRINTF(2, (stderr, "%s: Setting BIOCIMMEDIATE on %d\n",
			__func__, pcap_fd));
		if (ioctl(pcap_fd, BIOCIMMEDIATE, &on) < 0)
			warn("BIOCIMMEDIATE");
	}
#endif

	if (!interface_dopoll)
	{
		inter->if_recvev = event_new(libevent_base, pcap_fd, EV_READ, interface_recv, inter);
		event_add(inter->if_recvev, NULL);
	}
	else
	{
		struct timeval tv = HONEYD_POLL_INTERVAL;

		syslog(LOG_INFO, "switching to polling mode");
		inter->if_recvev = event_new(libevent_base, -1, EV_PERSIST, interface_poll_recv, inter);
		evtimer_add(inter->if_recvev, &tv);
	}
}

/*
 * Expands several command line arguments into a complete pcap filter string.
 * Deals with normal CIDR notation and IP-IP ranges.
 */

static char *
interface_expandips(int naddresses, char **addresses, int dstonly)
{
	static char filter[1024];
	char line[1024], *p;
	struct addr dst;

	if (naddresses == 0)
		return (NULL);

	filter[0] = '\0';

	while (naddresses--) {
		/* Get current address */
		p = *addresses++;

		if (filter[0] != '\0') {
			if (strlcat(filter, " or ", sizeof(filter)) >= sizeof(filter))
			{
				syslog(LOG_ERR, "%s: too many address for filter",__func__);
				exit(EXIT_FAILURE);
			}
		}

		/* XXX  addr_pton uses DNS and can block */
		if (addr_pton(p, &dst) != -1) {
			snprintf(line, sizeof(line), "%s%s%s",
			    dstonly ? "dst " : "",
			    dst.addr_bits != 32 ? "net ": "host ", p);
		} else {
			char *first, *second;
			struct addr astart, aend;
			struct in_addr in;
			ip_addr_t istart, iend;

			second = p;

			first = strsep(&second, "-");
			if (second == NULL)
			{
				syslog(LOG_ERR, "%s: Invalid network range: %s",__func__,p);
				exit(EXIT_FAILURE);
			}

			line[0] = '\0';
			if (addr_pton(first, &astart) == -1 ||
			    addr_pton(second, &aend) == -1)
			{
				syslog(LOG_ERR, "%s: bad addresses %s-%s", __func__, first, second);
				exit(EXIT_FAILURE);
			}
			if (addr_cmp(&astart, &aend) >= 0)
			{
				syslog(LOG_ERR, "%s: inverted range %s-%s", __func__, first, second);
				exit(EXIT_FAILURE);
			}

			/* Completely, IPv4 specific */
			istart = ntohl(astart.addr_ip);
			iend = ntohl(aend.addr_ip);
			while (istart <= iend) {
				char single[32];
				int count = 0, done = 0;
				ip_addr_t tmp;

				do {
					ip_addr_t bit = 1 << count;
					ip_addr_t mask;

					mask = ~(~0 << count);
					tmp = istart | mask;

					if (istart & bit)
						done = 1;

					if (iend < tmp) {
						count--;
						mask = ~(~0 << count);
						tmp = istart | mask;
						break;
					} else if (done)
						break;
					
					count++;
				} while (count < IP_ADDR_BITS);

				if (line[0] != '\0')
					strlcat(line, " or ", sizeof(line));
				in.s_addr = htonl(istart);
				snprintf(single, sizeof(single),
				    "dst net %s/%d",
				    inet_ntoa(in), 32 - count);

				strlcat(line, single, sizeof(line));

				istart = tmp + 1;
			}
		}
		
		if (strlcat(filter, line, sizeof(filter)) >= sizeof(filter))
		{
			syslog(LOG_ERR, "%s: too many address for filter", __func__);
			exit(EXIT_FAILURE);
		}
	}

	return (filter);
}

/* Interface receiving functions */

static void
interface_recv(int fd, short type, void *arg)
{
	struct interface *inter = arg;

	if (!interface_dopoll)
	{
		/*
		 * If the interface we are bound to goes down and comes back
		 * up, then we may enter in an infinite loop condition. The
		 * libevent library doesn't propagate the EPOLLERR/POLLERR to
		 * this code in any way, and versions of libpcap < 1.1.0 don't
		 * handle it gracefully either. So we read one byte here to
		 * clear the POLLERR condition if it exists.
		 */
		char c;

		if (recv(fd, &c, sizeof c, MSG_PEEK|MSG_DONTWAIT) == -1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				syslog(LOG_ERR, "recv error: %s", strerror(errno));
			}
		}
		event_add(inter->if_recvev, NULL);
	}

	if (pcap_dispatch(inter->if_pcap, -1, if_recv_cb, (u_char *)inter) < 0)
		syslog(LOG_ERR, "pcap_dispatch: %s",
		    pcap_geterr(inter->if_pcap));
}
 
static void
interface_poll_recv(int fd, short type, void *arg)
{
	interface_recv(fd, type, arg);
}

/* Unittests */
static void
interface_test_insert_and_find(void)
{
	struct interface *inter;
	struct addr tmp;

	if ((inter = calloc(1, sizeof(struct interface))) == NULL)
	{
		syslog(LOG_ERR, "%s: calloc", __func__);
		exit(EXIT_FAILURE);
	}

	addr_pton("10.0.0.254", &inter->if_ent.intf_addr);
	inter->if_addrbits = 24;
	strlcpy(inter->if_ent.intf_name, "fxp0",
	    sizeof(inter->if_ent.intf_name));

	TAILQ_INSERT_TAIL(&interfaces, inter, next);

	addr_pton("10.0.0.1", &tmp);
	if ( inter != interface_find_responsible(&tmp) )
	{
		syslog(LOG_ERR, "interface_find_responsibile failed");
		exit(EXIT_FAILURE);
	}
	if ( inter != interface_find("fxp0") )
	{
		syslog(LOG_ERR, "interface_find failed");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "\t%s: OK\n", __func__);
}

void
interface_test(void)
{
	interface_test_insert_and_find();
}
