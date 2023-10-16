/*
 * Copyright (c) 2002, 2003, 2004 Niels Provos <provos@citi.umich.edu>
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

#include "config.h"
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <sys/stat.h>
#include <sys/tree.h>
#include <sys/queue.h>
#include <sys/socket.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <dnet.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <fnmatch.h>

#include <pcap.h>

#undef timeout_pending
#undef timeout_initialized

#include <event.h>

#include "honeyd.h"
#include "personality.h"
#include "template.h"
#include "subsystem.h"
#include "condition.h"
#include "interface.h"
#include "parser.h"
#include "ethernet.h"
#include "arp.h"
#include "pool.h"
#include "dhcpclient.h"
#include "util.h"
#include "log.h"

/* Tailq that holds all subsystems */
struct subsystemqueue subsystems;

/* Tree that contains all templates */
struct templtree templates;

/* Counter for addresses in 169.254/16 that we assign for DHCP */
static uint16_t privip_counter = 1;

int
templ_compare(struct template *a, struct template *b)
{
	return (strcmp(a->name, b->name));
}

SPLAY_GENERATE(templtree, template, node, templ_compare);

int
port_compare(struct port *a, struct port *b)
{
	int diff;

	diff = a->proto - b->proto;
	if (diff) {
		return (diff);
	} else {
		/* safe because number in uint16_t */
		return ((int)a->number - (int)b->number);
	}
}

SPLAY_PROTOTYPE(porttree, port, node, port_compare);
SPLAY_GENERATE(porttree, port, node, port_compare);



void
config_init(void)
{
	TAILQ_INIT(&subsystems);
	SPLAY_INIT(&templates);

	no_spoof.new_src.addr_type = ADDR_TYPE_NONE;	/* default is no source spoofing... */
	no_spoof.new_dst.addr_type = ADDR_TYPE_NONE;	/* ... and no destination spoofing */
}

void
config_read(char *config)
{
	extern int honeyd_ignore_parse_errors;
	FILE *fp;

	if ((fp = fopen(config, "r")) == NULL)
	{
		syslog(LOG_ERR, "fopen(%s)",config);
		exit(EXIT_FAILURE);
	}
	if (parse_configuration(fp, config) == -1 &&
	    !honeyd_ignore_parse_errors)
	{
		syslog(LOG_ERR, "%s: parsing configuration file failed",__func__);
		exit(EXIT_FAILURE);
	}
	fclose(fp);
}

struct template *
template_find(const char *name)
{
	struct template tmp;

	tmp.name = (char *)name;
	return (SPLAY_FIND(templtree, &templates, &tmp));
}

// Dumps IP addresses of DHCP
void template_dump_ips(char* filePath)
{
	FILE *fp;

	if ((fp = fopen(filePath , "w+")) == NULL)
	{
		syslog(LOG_WARNING, "Error opening the DHCP IP address dump file");
		//warn("Error opening the DHCP IP address dump file");
		return;
	}

	struct template *tmpl;
	SPLAY_FOREACH(tmpl, templtree, &templates)
	{
		if((tmpl->dhcp_req != NULL))
		{
			if(tmpl->dhcp_req->state == DHREQ_STATE_GOTACK)
			{
				char hwAddrString[19];
				memset(hwAddrString, '\0', sizeof(hwAddrString));
				//Only write to the size of the array-1, so that we're sure there's a NULL byte at the end
				if(addr_ntop(tmpl->ethernet_addr, hwAddrString, sizeof(hwAddrString) -1) != NULL)
				{
					fprintf(fp, "%s, %s, %s\n", tmpl->name, hwAddrString, tmpl->inter->if_ent.intf_name);
				}
			}
		}
	}
	fclose(fp);
}

void
template_list_glob(struct evbuffer *buffer, const char *pattern)
{
	struct template *tmpl, *last = NULL;
	struct evbuffer *tmp = NULL;
	int count = 0;

	tmpl = template_find(pattern);
	if (tmpl != NULL) {
		template_print(buffer, tmpl);
		return;
	}	

	if ((tmp = evbuffer_new()) == NULL)
		{
		syslog(LOG_ERR, "%s: malloc",__func__);
		exit(EXIT_FAILURE);
		}

	SPLAY_FOREACH(tmpl, templtree, &templates) {
		/* Ignore it if it does not match */
		if (fnmatch(pattern, tmpl->name, 0))
			continue;
		count++;
		evbuffer_add_printf(tmp, "%4d. %s (%s)\n",
		    count,
		    tmpl->name,
		    tmpl->person != NULL ? tmpl->person->name : "undefined");

		last = tmpl;
	}

	if (count == 1) {
		template_print(buffer, last);
	} else {
		evbuffer_add_buffer(buffer, tmp);
	}
	evbuffer_free(tmp);
}

/*
 * Checks if condition for each template in the list is matched.
 * Return the first template that we match.
 */

struct template *
template_dynamic(const struct template *tmpl, const struct ip_hdr *ip,
    u_short iplen)
{
	struct template *save = NULL;
	struct condition *cond;

	TAILQ_FOREACH(cond, &tmpl->dynamic, next) {
		if (save == NULL)
			save = cond->tmpl;

		/* See if we match this template and return it on success */
		if (cond->match == NULL ||
		    cond->match(cond->tmpl, ip, iplen, cond->match_arg))
			return (cond->tmpl);
	}

	/* We need to return something, so return the first non-NULL */
	return (save);
}

struct template *
template_find_best(const char *addr, const struct ip_hdr *ip, u_short iplen)
{
	struct template *tmpl;

	tmpl = template_find(addr);
	if (tmpl == NULL)
		tmpl = template_find("default");
	
	if (tmpl != NULL && tmpl->flags & TEMPLATE_DYNAMIC)
		tmpl = template_dynamic(tmpl, ip, iplen);

	return (tmpl);
}

struct template *
template_create(const char *name)
{
	extern rand_t *honeyd_rand;
	struct template *tmpl;

	if (template_find(name))
		return (NULL);

	if ((tmpl = calloc(1, sizeof(struct template))) == NULL)
		{
			syslog(LOG_ERR, "%s: calloc",__func__);
			exit(EXIT_FAILURE);
		}

	tmpl->name = strdup(name);

	/* UDP ports are closed by default */
	tmpl->udp.status = PORT_CLOSED;

	SPLAY_INIT(&tmpl->ports);
	SPLAY_INIT(&tmpl->bcasts);
	SPLAY_INSERT(templtree, &templates, tmpl);

	/* Configured subsystems */
	TAILQ_INIT(&tmpl->subsystems);
	TAILQ_INIT(&tmpl->dynamic);

	/* No spoofing, by default */
	tmpl->spoof = no_spoof;

	/* Crank ref counter */
	tmpl->refcnt++;

	/* Create a drift */
	tmpl->drift = 1.0 + (rand_uint16(honeyd_rand) % 140 - 70)/100000.0;

	return (tmpl);
}

int
template_iterate(int (*f)(struct template *, void *), void *arg)
{
	struct template *tmpl;

	SPLAY_FOREACH(tmpl, templtree, &templates) {
		if ((*f)(tmpl, arg) == -1)
			return (-1);
	}

	return (0);
}

/*
 * Removes all configured templates from the system, so that the
 * configuration file can be re-read.
 */

void
template_free_all(int how)
{
	struct template *tmpl;

	while ((tmpl = SPLAY_ROOT(&templates)) != NULL) {
		SPLAY_REMOVE(templtree, &templates, tmpl);
		if (how == TEMPLATE_FREE_REGULAR)
			template_free(tmpl);
		else if (!(tmpl->flags & TEMPLATE_DYNAMIC_CHILD))
			template_deallocate(tmpl);
	}
}

/* Remove a template from the system */

void
template_remove(struct template *tmpl)
{
	/* Remove ourselves from the searchable index */
	if (template_find(tmpl->name) == tmpl)
		SPLAY_REMOVE(templtree, &templates, tmpl);
}

/* Insert a template into the system */

int
template_insert(struct template *tmpl)
{
	/* Insert ourselves into the searchable index */
	if (template_find(tmpl->name) != NULL)
		return (-1);
	SPLAY_INSERT(templtree, &templates, tmpl);


	struct port *bport;

	// Start broadcasts from this honeypot
	// TODO refactor this to a template_activate function
	if (tmpl->honeypot_instance)
	{
		SPLAY_FOREACH(bport, porttree, (struct porttree *)&tmpl->bcasts) {
			struct timeval every;
			every.tv_sec = bport->timeout;
			every.tv_usec = 0;

			strncpy(bport->templateName, tmpl->name, sizeof(bport->templateName));

			struct udp_con *con = calloc(1, sizeof(struct udp_con));
			con->port = bport;


			struct event *ev1 = event_new(libevent_base, -1, EV_TIMEOUT | EV_PERSIST, &bcast_trigger, (void*)con);
			event_add(ev1, &every);

			// TODO get rid of event when template destructed and garbage collect the ports
		}
	}

	return (0);
}

void
template_deallocate(struct template *tmpl)
{
	struct condition *cond;
	struct port *port;

	/* Remove ourselves from the searchable index */
	if (template_find(tmpl->name) == tmpl)
		SPLAY_REMOVE(templtree, &templates, tmpl);

	/* Free conditions for dynamic templates */
	for (cond = TAILQ_FIRST(&tmpl->dynamic); cond != NULL;
	    cond = TAILQ_FIRST(&tmpl->dynamic)) {
		TAILQ_REMOVE(&tmpl->dynamic, cond, next);

		template_free(cond->tmpl);
		if (cond->match_arg)
			free(cond->match_arg);
		free(cond);
	}
	
	/* Remove ports from template */
	while ((port = SPLAY_ROOT(&tmpl->ports)) != NULL)
		port_free(tmpl, port);

	if (tmpl->person != NULL)
		personality_declone(tmpl->person);

	if (tmpl->ethernet_addr != NULL) {
		struct arp_req *req = arp_find(tmpl->ethernet_addr);
		/*
		 * Templates that are not bound to IP addresses do not
		 * have an associated arp request object.
		 */
		if (req != NULL)
			arp_free(req);
		free(tmpl->ethernet_addr);
	}

	if (tmpl->dhcp_req != NULL) {
		dhcp_release(tmpl);
		dhcp_abort(tmpl);
		free(tmpl->dhcp_req);
	}

	free(tmpl->name);
	free(tmpl);
}

struct port *
port_find(struct template *tmpl, int proto, int number)
{
	struct port tmpport;
	
	tmpport.proto = proto;
	tmpport.number = number;
	
	return (SPLAY_FIND(porttree, &tmpl->ports, &tmpport));
}

void
port_action_clone(struct action *dst, const struct action *src)
{
	*dst = *src;
	if (src->action) {
		dst->action = strdup(src->action);
		if (dst->action == NULL)
		{
			syslog(LOG_ERR, "%s: strdup",__func__);
			exit(EXIT_FAILURE);
		}
	}

	if (src->aitop != NULL) {
		struct addrinfo *ai = src->aitop;
		char addr[NI_MAXHOST];
		char port[NI_MAXSERV];
		short nport;

		if (getnameinfo(ai->ai_addr, ai->ai_addrlen,
			addr, sizeof(addr), port, sizeof(port),
			NI_NUMERICHOST|NI_NUMERICSERV) != 0)
		{
			syslog(LOG_ERR, "%s: getnameinfo", __func__);
			exit(EXIT_FAILURE);
		}
		nport = atoi(port);
		dst->aitop = cmd_proxy_getinfo(addr, ai->ai_socktype, nport);
		if (dst->aitop == NULL)
		{
			syslog(LOG_ERR, "%s: cmd_proxy_getinfo failed", __func__);
			exit(EXIT_FAILURE);
		}
	}
}

void
port_encapsulation_free(struct port_encapsulate *tmp)
{
	struct port *port = tmp->port;

	TAILQ_REMOVE(&port->pending, tmp, next);
	event_del(tmp->ev);
	
	/* Remove the reference to the pending connection */
	if (tmp->hdr != NULL)
		tmp->hdr->pending = NULL;

	free(tmp);
}

void
port_free(struct template *tmpl, struct port *port)
{
	struct port_encapsulate *tmp;

	/* Remove pending connections */
	while ((tmp = TAILQ_FIRST(&port->pending)) != NULL) {
		/* This might not be the correct way to clean this up */
		if (tmp->hdr != NULL && tmp->hdr->type == SOCK_STREAM)
			tcp_connectfail(tmp->con);
	
		port_encapsulation_free(tmp);
	}

	SPLAY_REMOVE(porttree, &tmpl->ports, port);

	

	if (port->sub_conport != NULL) {
		/* Back pointer to connection object.
		 * It allows us to remove the reference to this object
		 * in the connection.
		 * However, at this point we really need to tear down
		 * that connection, too.
		 */
		*port->sub_conport = NULL;
	}

	if (port->sub != NULL)
		TAILQ_REMOVE(&port->sub->ports, port, next);
	if (port->sub_fd != -1)
		fdshare_close(port->sub_fd);
	if (port->action.action != NULL)
		free (port->action.action);
	if (port->action.aitop != NULL)
		freeaddrinfo(port->action.aitop);
	free(port);
}

// Callback for honeyd script broadcasts
void bcast_trigger(int fd, short what, void *ptr) {
	int i;
	struct udp_con *con = (struct udp_con*)ptr;
	struct port *bport = con->port;
	struct template *tmpl = template_find((const char *)&bport->templateName);

	struct in_addr honeypotAddress;
	inet_aton(tmpl->name, &honeypotAddress);

	struct ip_hdr ip;
	struct udp_hdr udp;

	ip.ip_src = tmpl->inter->subnetBcastAddress;

	ip.ip_dst = honeypotAddress.s_addr;
	udp.uh_sport = htons(bport->number);
	udp.uh_dport = htons(bport->srcport);

	struct udp_con *udpcon = udp_new(&ip, &udp, 0);
	udpcon->tmpl = template_ref(tmpl);;

	char *argv[32];

	/* Create arguments */
	char *p, *p2;
	char line[512];

	strlcpy(line, bport->action.action, sizeof(line));
	p2 = line;
	for (i = 0; i < sizeof(argv)/sizeof(char *) - 1; i++) {
		if ((p = strsep(&p2, " ")) == NULL)
			break;
		if (strlen(p) == 0) {
			i--;
			continue;
		}

		argv[i] = p;
	}

	argv[i] = NULL;

	cmd_fork(&udpcon->conhdr, &udpcon->cmd, tmpl, argv[0], argv, (void*)udpcon);
}

// This lets you have a honeyd script run every n seconds and send out a broadcast UDP packet
void bcast_insert(struct template *tmpl, int srcport, int dstport, int seconds, struct action *action)
{
	struct port *p;
	p = calloc(1, sizeof(struct port));
	p->action = *action;
	p->number = dstport;
	p->srcport = srcport;
	p->timeout = seconds;

	SPLAY_INSERT(porttree, &tmpl->bcasts, p);
}

struct port *
port_insert(struct template *tmpl, int proto, int number,
    struct action *action)
{
	struct port *port, tmpport;
	
	tmpport.proto = proto;
	tmpport.number = number;
	
	// Check if this port/protocol already has an action set
	if (SPLAY_FIND(porttree, &tmpl->ports, &tmpport) != NULL)
	{
		syslog(LOG_WARNING, "WARNING: replacing already defined port/protocol for template %s: port: %d | protocol: %d", tmpl->name, number, proto);
	}
	
	if ((port = calloc(1, sizeof(struct port))) == NULL)
	{
		syslog(LOG_ERR, "%s: calloc", __func__);
		exit(EXIT_FAILURE);
	}

	TAILQ_INIT(&port->pending);
	port->sub = NULL;
	port->sub_fd = -1;
	port->proto = proto;
	port->number = number;
	port_action_clone(&port->action, action);
	    
	SPLAY_INSERT(porttree, &tmpl->ports, port);

	return (port);
}

/* Create a random port in a certain range */

struct port *
port_random(struct template *tmpl, int proto, struct action *action,
    int min, int max)
{
	extern rand_t *honeyd_rand;
	struct port *port = NULL;
	int count = 100;
	int number;

	while (count-- && port == NULL) {
		number = rand_uint16(honeyd_rand) % (max - min) + min;
		port = port_insert(tmpl, proto, number, action);
	}

	return (port);
}

int
template_add(struct template *tmpl, int proto, int number,
    struct action *action)
{
	return (port_insert(tmpl, proto, number, action) == NULL ? -1 : 0);
}

void
template_insert_subsystem(struct template *tmpl, struct subsystem *sub)
{
	struct subsystem_container *container;

	if ((container = malloc(sizeof(struct subsystem_container))) == NULL)
	{
		syslog(LOG_ERR, "%s: malloc", __func__);
		exit(EXIT_FAILURE);
	}

	container->sub = sub;
	TAILQ_INSERT_TAIL(&tmpl->subsystems, container, next);
}

/* This function is slow, but should only called on SIGHUP */

void
template_remove_subsystem(struct template *tmpl, struct subsystem *sub)
{
	struct subsystem_container *container;

	TAILQ_FOREACH(container, &tmpl->subsystems, next) {
		if (container->sub == sub)
			break;
	}

	if (container == NULL)
	{
		syslog(LOG_ERR, "%s: could not remove subsystem %p from %s",__func__, sub, tmpl->name);
		exit(EXIT_FAILURE);
	}

	TAILQ_REMOVE(&tmpl->subsystems, container, next);

	free(container);
}

void
template_post_arp(struct template *tmpl, struct addr *ipaddr)
{
	struct arp_req *req;

	/* Register this mac address as our own */
	req = arp_new(tmpl->inter, NULL, NULL, ipaddr, tmpl->ethernet_addr);
	if (req == NULL)
	{
		syslog(LOG_ERR, "%s: cannot create arp entry",__func__);
		exit(EXIT_FAILURE);
	}
		
	req->flags |= ARP_INTERNAL;
	req->owner = tmpl;
}

void
template_remove_arp(struct template *tmpl)
{
	struct arp_req *arp;

	if (tmpl->ethernet_addr == NULL)
		return;

	/* Check if we have an ARP entry */
	arp = arp_find(tmpl->ethernet_addr);
	assert(arp != NULL);
	arp_free(arp);
}

/*
 * When the interface is specified, we do not need to do an address lookup
 * for a corresponding interface.  We use the interface for DHCP.
 */

struct template *
template_clone(const char *newname, const struct template *tmpl, 
    struct interface *inter, int start)
{
	struct subsystem_container *container;
	struct condition *condition;
	struct template *newtmpl;
	struct port *port;
	struct in_addr in_addr;

	if ((newtmpl = template_create(newname)) == NULL)
		return (NULL);

	SPLAY_FOREACH(port, porttree, (struct porttree *)&tmpl->ports) {
		if (port_insert(newtmpl, port->proto, port->number,
			&port->action) == NULL)
			return (NULL);
	}

	SPLAY_FOREACH(port, porttree, (struct porttree *)&tmpl->bcasts) {
		bcast_insert(newtmpl, port->srcport, port->number, port->timeout,&port->action);
	}

	if(tmpl == NULL)
		return NULL;

	if (tmpl->person)
		newtmpl->person = personality_clone(tmpl->person);

	if (tmpl->ethernet_addr) {
		newtmpl->ethernet_addr = ethernetcode_clone(tmpl->ethernet_addr);
		/* 
		 * This template wants to have its own ethernet address,
		 * so we need to register it with our arp code.
		 *
		 * DHCP templates get a temporary IP address assigned.
		 */

		//If the template name is a valid IP address
		if(inet_pton(AF_INET, newtmpl->name, &in_addr) == 1)
		{
			//convert struct in_addr to addr
			struct sockaddr_in tmpSockaddr;
			struct addr addr;

			tmpSockaddr.sin_family = AF_INET;
			tmpSockaddr.sin_addr = in_addr;
			tmpSockaddr.sin_port = 0;	//There is no socket yet. No ports.

			addr_ston((struct sockaddr*)&tmpSockaddr, &addr);

			if (inter == NULL)
			{
				inter = interface_find_responsible(&addr);
				if (inter == NULL)
				{
					syslog(LOG_ERR, "Cannot find interface");
					exit(EXIT_FAILURE);
				}
			}
			newtmpl->inter = inter;

			/* Register this mac address as our own */
			template_post_arp(newtmpl, &addr);

			newtmpl->honeypot_instance = 1;
		} else {
			newtmpl->honeypot_instance = 0;
		}
	}

	port_action_clone(&newtmpl->tcp, &tmpl->tcp);
	port_action_clone(&newtmpl->udp, &tmpl->udp);
	port_action_clone(&newtmpl->icmp, &tmpl->icmp);

	newtmpl->timestamp = tmpl->timestamp;
	newtmpl->uid = tmpl->uid;
	newtmpl->gid = tmpl->gid;
	newtmpl->max_nofiles = tmpl->max_nofiles;
	newtmpl->drop_inrate = tmpl->drop_inrate;
	newtmpl->drop_synrate = tmpl->drop_synrate;
	newtmpl->flags = tmpl->flags;
	newtmpl->spoof = tmpl->spoof;
	newtmpl->addrbits = tmpl->addrbits;
	newtmpl->honeypot_instance = tmpl->honeypot_instance;

	/* We need to remove this when cloning */
	newtmpl->flags &= ~TEMPLATE_DYNAMIC_CHILD;

	/* Clone dynamics */
	TAILQ_FOREACH(condition, &tmpl->dynamic, next) {
		if (template_insert_dynamic(newtmpl,
			condition->tmpl, condition) == -1)
			warn("%s: couldn't insert dynamic cond %s into %s",
			    condition->tmpl->name, tmpl->name);
	}

	/* Clone subsystems */
	TAILQ_FOREACH(container, &tmpl->subsystems, next) {
		struct subsystem *sub = container->sub;

		/* 
		 * Create a new subsystem structure only if the
		 * subsystem has not been specified as shared in the
		 * configuration.
		 */
		if (!(sub->flags & SUBSYSTEM_SHARED)) {
			template_subsystem(newtmpl, sub->cmdstring,sub->flags);
			continue;
		}

		/* Otherwise, just create references */
		subsystem_insert_template(sub, newtmpl);

		template_insert_subsystem(newtmpl, sub);
	}

	/* Start subsystems if we are the master template for a subsystem */
	if (!start)
		return (newtmpl);

	/* Start background processes */
	TAILQ_FOREACH(container, &newtmpl->subsystems, next) {
		template_subsystem_start(newtmpl, container->sub);
	}

	return (newtmpl);
}

int
template_subsystem(struct template *tmpl, char *subsystem, int flags)
{
	struct subsystem *sub;
	
	if ((sub = calloc(1, sizeof(struct subsystem))) == NULL)
	{
		syslog(LOG_ERR, "%s: calloc", __func__);
		exit(EXIT_FAILURE);
	}

	if ((sub->cmdstring = strdup(subsystem)) == NULL)
	{
		syslog(LOG_ERR, "%s: strdup", __func__);
		exit(EXIT_FAILURE);
	}

	/* Initializes subsystem data structures */
	TAILQ_INIT(&sub->ports);
	SPLAY_INIT(&sub->root);
	TAILQ_INIT(&sub->templates);

	subsystem_insert_template(sub, tmpl);

	sub->cmd.pid = -1;
	sub->flags |= flags;

	template_insert_subsystem(tmpl, sub);

	/* Remember this subsystem for lookup through the UI */
	TAILQ_INSERT_TAIL(&subsystems, sub, next);

	return (0);
}

void
template_subsystem_free_ports(struct subsystem *sub)
{
	struct port *port;

	for (port = TAILQ_FIRST(&sub->ports); port;
	    port = TAILQ_FIRST(&sub->ports)) {
		
		/* Free all ports for this subsystem */
		port_free(port->subtmpl, port);
	}
}

struct subsystem *
template_subsystem_find(const char *name)
{
	struct subsystem *sub;

	TAILQ_FOREACH(sub, &subsystems, next) {
		if (!strcmp(name, sub->cmdstring))
			return (sub);
	}

	return (NULL);
}

void
template_subsystem_list_glob(struct evbuffer *buffer, const char *pattern)
{
	struct subsystem *sub, *last = NULL;
	struct evbuffer *tmp = NULL;
	int count = 0;

	sub = template_subsystem_find(pattern);
	if (sub != NULL) {
		subsystem_print(buffer, sub);
		return;
	}

	if ((tmp = evbuffer_new()) == NULL)
	{
		syslog(LOG_ERR, "%s: malloc",__func__);
		exit(EXIT_FAILURE);
	}

	TAILQ_FOREACH(sub, &subsystems, next) {
		/* Ignore it if it does not match */
		if (fnmatch(pattern, sub->cmdstring, 0))
			continue;

		count++;
		evbuffer_add_printf(tmp, "%4d. %s (%d)\n",
		    count, sub->cmdstring, sub->cmd.pid);

		last = sub;
	}

	if (count == 1) {
		subsystem_print(buffer, last);
	} else {
		evbuffer_add_buffer(buffer, tmp);
	}
	evbuffer_free(tmp);
}

void
template_subsystem_free(struct subsystem *sub)
{
	struct template_container *cont;
	struct template *tmpl;

	TAILQ_REMOVE(&subsystems, sub, next);

	template_subsystem_free_ports(sub);
		
	/* 
	 * As we are removing all templates for this subsystem, we
	 * actually do not need to remove them from the Splay.
	 */
	for (cont = TAILQ_FIRST(&sub->templates); cont;
	    cont = TAILQ_FIRST(&sub->templates)) {
		TAILQ_REMOVE(&sub->templates, cont, next);
		tmpl = cont->tmpl;
		free(cont);

		template_remove_subsystem(tmpl, sub);

		template_free(tmpl);
	}

	cmd_free(&sub->cmd);

	free(sub->cmdstring);
	free(sub);
}

/* 
 * Inserts a new conditional template into a dynamic template.
 * The condition gets cloned in the process.
 */

int
template_insert_dynamic(struct template *tmpl, struct template *child,
    struct condition *condition)
{
	char newname[1024];
	struct condition *cond;

	if ((cond = calloc(1, sizeof(struct condition))) == NULL)
	{
		syslog(LOG_ERR, "%s: calloc", __func__);
		exit(EXIT_FAILURE);
	}

	if (condition != NULL)
		*cond = *condition;

	/* Con up a new template name */
	snprintf(newname, sizeof(newname), "%s_%d_%s",
	    tmpl->name, tmpl->dynamic_rulenr++, child->name);
	if ((cond->tmpl = template_clone(newname, child, NULL, 0)) == NULL) {
		fprintf(stderr, "Failed to clone %s from %s\n",
		   newname, child->name);
		free(cond);
		return (-1);
	}
	cond->tmpl->flags |= TEMPLATE_DYNAMIC_CHILD;

	TAILQ_INSERT_TAIL(&tmpl->dynamic, cond, next);

	/* Do we need to copy the match arg, too? */
	if (condition == NULL || condition->match_arg == NULL)
		return (0);

	if ((cond->match_arg = malloc(cond->match_arglen)) == NULL)
	{
		syslog(LOG_ERR, "%s: malloc", __func__);
		exit(EXIT_FAILURE);
	}

	memcpy(cond->match_arg, condition->match_arg, cond->match_arglen);

	return (0);
}

int
template_get_dhcp_address(struct addr *addr)
{
	static char address[16];

	snprintf(address, sizeof(address),
	    "169.254.%d.%d", privip_counter / 256 + 1, privip_counter % 256);

	if (++privip_counter > 255 * 255)
	{
		syslog(LOG_ERR, "%s: out of temporary IP addresses", __func__);
		exit(EXIT_FAILURE);
	}

	return (addr_aton(address, addr));
}

void
template_subsystem_start(struct template *tmpl, struct subsystem *sub)
{
	extern int honeyd_verify_config;
	char *p;
	char *argv[4];
	char line[512];	/* for command replacement */
	char *name = tmpl->name;
	struct addr addr;
	int isipaddr = addr_aton(name, &addr) != -1;


	/* Has the subsystem started already? */
	if (sub->cmd.pid != -1 || honeyd_verify_config)
		return;

	gettimeofday(&sub->tv_restart, NULL);

	argv[0] = "/bin/sh";
	argv[1] = "-c";
	argv[3] = NULL;

	strlcpy(line, sub->cmdstring, sizeof(line));
	if (isipaddr) {
		/*
		 * Provide a subsystem with it's own IP address.
		 * For a shared subsystem, this is going to fail.
		 */
		while (strrpl(line, sizeof(line), "$ipsrc", name) != NULL)
			;
	}
	p = honeyd_logdate();
        while (strrpl(line, sizeof(line), "$date", p) != NULL)
                ;

	argv[2] = line;
	if (cmd_subsystem(tmpl, sub, "/bin/sh", argv) == -1)
	{
		syslog(LOG_ERR, "%s: can not start subsystem \"%s\" for %s",__func__ ,sub->cmdstring, name);
		exit(EXIT_FAILURE);
	}

}

void
template_print(struct evbuffer *buffer, struct template *tmpl)
{
	struct port *port;

	evbuffer_add_printf(buffer, "template %s:\n", tmpl->name);
	evbuffer_add_printf(buffer, "  personality: %s\n",
	    tmpl->person != NULL ? tmpl->person->name : "undefined");
	if (tmpl->ethernet_addr != NULL)
		evbuffer_add_printf(buffer, "  ethernet address: %s\n",
		    addr_ntoa(tmpl->ethernet_addr));
	evbuffer_add_printf(buffer, "  IP id: %u\n", tmpl->ipid);
	evbuffer_add_printf(buffer, "  TCP seq: %x\n", tmpl->seq);
	evbuffer_add_printf(buffer, "  TCP drop: in: %d syn: %d\n",
	    tmpl->drop_inrate, tmpl->drop_synrate);
	evbuffer_add_printf(buffer, "  refcnt: %d\n", tmpl->refcnt);
	evbuffer_add_printf(buffer, "  ports:\n");

	SPLAY_FOREACH(port, porttree, &tmpl->ports) {
		char *type;
		switch (port->action.status) {
		case PORT_OPEN:
			type = "open";
			break;
		case PORT_PROXY:
			type = "proxy";
			break;
		case PORT_FILTERED:
			type = "filtered";
			break;
		case PORT_CLOSED:
			type = "closed";
			break;
		case PORT_SUBSYSTEM:
			type = "subsystem";
			break;
		case PORT_PYTHON:
			type = "python";
			break;
		default:
			type = "reserved";
			break;
		}
		evbuffer_add_printf(buffer, "    %s %5d %s",
		    port->proto == IP_PROTO_TCP ? "tcp" : "udp",
		    port->number, type);
		evbuffer_add_printf(buffer, "\n");
		if (port->action.status == PORT_SUBSYSTEM) {
			evbuffer_add_printf(buffer, "\t%s\n",
			    port->sub->cmdstring);
		} else if (port->action.status == PORT_OPEN &&
		    port->action.action != NULL) {
			evbuffer_add_printf(buffer, "\t%s\n",
			    port->action.action);
		}
	}
}

/***************************************************************************
 * Everything is unittest related below this
 ***************************************************************************/

void
template_delay_cb(int fd, short which, void *arg)
{
	extern struct pool *pool_pkt;
	extern struct pool *pool_delay;
	struct delay *delay = arg;
	struct ip_hdr *ip = delay->ip;
	struct template *tmpl = delay->tmpl;
	u_int iplen = delay->iplen;

	if (ip->ip_ttl &&
	    !(delay->flags & (DELAY_UNREACH|DELAY_EXTERNAL|DELAY_TUNNEL|DELAY_ETHERNET))) {
		struct addr addr;

		template_free(tmpl);

		addr_pack(&addr, ADDR_TYPE_IP, IP_ADDR_BITS,
		    &ip->ip_dst, IP_ADDR_LEN);

		/* Internal delivery */
		tmpl = template_find_best(addr_ntoa(&addr), ip, iplen);
		tmpl = template_ref(tmpl);

		/* No Check for fragmentation */
		honeyd_dispatch(tmpl, NULL, ip, iplen);

	}

	if (delay->flags & DELAY_FREEPKT)
		pool_free(pool_pkt, ip);
	template_free(tmpl);

	if (delay->flags & DELAY_NEEDFREE)
		pool_free(pool_delay, delay);
}

void
template_test_parse_error(char *line, struct evbuffer *evbuf)
{
	char *p = (char*)evbuffer_pullup(evbuf, -1);
	size_t off = evbuffer_get_length(evbuf);
	p[off - 1] = '\0';
	syslog(LOG_ERR, "parse_line \"%s\" failed: %s",line,p);
	exit(EXIT_FAILURE);
}

#define MAKE_CONFIG(x)	do { \
	char *p = (x); \
	if (parse_line(evbuf, p) == -1) \
		template_test_parse_error(p, evbuf); \
} while (0)
	
int
template_test_add(struct evbuffer *evbuf, struct addr *addr, int count)
{
	char line[128];
	int i;

	for (i = 0; i < count; i++) {
		evbuffer_drain(evbuf, -1);
		
		addr->addr_ip = htonl(ntohl(addr->addr_ip) + 1);
		snprintf(line, sizeof(line), "bind %s template",
		    addr_ntoa(addr));
		MAKE_CONFIG(line);
	}

	return (count);
}

void
template_test_measure(int count)
{
	extern rand_t *honeyd_rand;
	extern void
	  honeyd_recv_cb(u_char *, const struct pcap_pkthdr *, const u_char *);
	u_char pkt[1500];
	struct interface inter;
	struct pcap_pkthdr pkthdr;
	struct ip_hdr *ip = (struct ip_hdr *)pkt;
	struct tcp_hdr *tcp = (struct tcp_hdr *)(ip + 1);
	struct addr src, dst;
	uint16_t *snib;
	struct timeval tv_start, tv_end;
	double msperpkt;
	int j;
	uint16_t iplen = sizeof(*ip) + sizeof(*tcp);

	memset(&inter, 0, sizeof(struct interface));
	memset(&pkthdr, 0, sizeof(struct pcap_pkthdr));
	pkthdr.caplen = sizeof(pkt);
		    
	addr_pton("10.0.0.0", &dst);
	addr_pton("10.1.0.0", &src);

	snib = (uint16_t *)(&src.addr_data8[2]);

	gettimeofday(&tv_start, NULL);
	for (j = 0; j < 80000; j++) {
		*snib = rand_uint16(honeyd_rand);
		dst.addr_ip  = htonl(rand_uint32(honeyd_rand) % count);
		dst.addr_data8[0] = 10;

		tcp_pack_hdr(tcp,
		    rand_uint16(honeyd_rand), 23,
		    0x100000, 0x00000, TH_SYN, 32768, 0);
		ip_pack_hdr(pkt, 0, iplen, rand_uint16(honeyd_rand),
		    0, 64,
		    IP_PROTO_TCP, src.addr_ip, dst.addr_ip);
		ip_checksum(pkt, iplen);

		honeyd_recv_cb((u_char *)&inter, &pkthdr, pkt);
	}
	gettimeofday(&tv_end, NULL);
	timersub(&tv_end, &tv_start, &tv_end);
	msperpkt = (double)(tv_end.tv_sec * 1000 + tv_end.tv_usec / 1000)
	    / (double) j;
	syslog(LOG_ERR, "\t\t%7d templates: %.4f ms per packet\n", count,msperpkt);
	fprintf(stderr, "\t\t%7d templates: %.4f ms per packet\n",
	    count, msperpkt);
}

void
template_packet_test(void)
{
	extern void (*honeyd_delay_callback)(int, short, void *);
	void (*old)(int, short, void *) = honeyd_delay_callback;
	struct evbuffer *evbuf = evbuffer_new();
	struct addr addr;
	int i, count;

	/* Set our own delay callback */
	honeyd_delay_callback = template_delay_cb;

	/* Create configuration */
	MAKE_CONFIG("create template");
	MAKE_CONFIG("add template tcp port 23 closed");

	addr_pton("10.0.0.0", &addr);

	count = template_test_add(evbuf, &addr, 1);
	template_test_measure(count);
	for (i = 0; i < 5; i++) {
		count += template_test_add(evbuf, &addr,
		    count == 1 ? 999 : 1000);
		template_test_measure(count);
	}
	for (i = 0; i < 50; i++) {
		count += template_test_add(evbuf, &addr, 5000);
		template_test_measure(count);
	}

	honeyd_delay_callback = old;

	evbuffer_free(evbuf);

	fprintf(stderr, "\t%s: OK\n", __func__);
}

void
template_test(void)
{
	setlogmask(LOG_UPTO(LOG_NOTICE));

	template_packet_test();
}
