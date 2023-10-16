/*
 * Copyright (c) 2004 Niels Provos <provos@citi.umich.edu>
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
#include <stdint.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/tree.h>
#include <syslog.h>

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <ctype.h>

#include <dnet.h>

#include "ethernet.h"

struct etherindex {
	SPLAY_ENTRY(etherindex) node;

	char *index_word;

	struct ethernetcode **list;
	size_t list_size;
	size_t list_mem;
};

struct ethernetcode {
	uint32_t prefix;
	char *vendor;
	int count;
};

static SPLAY_HEAD(ethertree, etherindex) etherroot;

static int
compare(struct etherindex *a, struct etherindex *b)
{
	return (strcmp(a->index_word, b->index_word));
}

SPLAY_PROTOTYPE(ethertree, etherindex, node, compare);

SPLAY_GENERATE(ethertree, etherindex, node, compare);

static int
ethernetcode_index(struct ethertree *etherroot, struct ethernetcode *code)
{
	struct etherindex tmp, *entry;
	char line[1024], *p, *e;


	strlcpy(line, code->vendor, sizeof(line));
	e = line;

	/* Walk through every single word and index it */
	while ((p = strsep(&e, " ")) != NULL) {
		tmp.index_word = p;
		if ((entry = SPLAY_FIND(ethertree, etherroot, &tmp)) == NULL) {
			/* Generate a new entry for this word */
			entry = calloc(1, sizeof(struct etherindex));
			if (entry == NULL)
			{
				syslog(LOG_ERR, "%s: calloc, failed to allocate new entry for the current word", __func__);
				exit(EXIT_FAILURE);
			}

			if ((entry->index_word = strdup(p)) == NULL)
			{
				syslog(LOG_ERR, "%s: strdup", __func__);
				exit(EXIT_FAILURE);
			}

			entry->list_mem = 32;
			if ((entry->list = calloc(entry->list_mem,
					sizeof(struct ethernetcode *))) == NULL)
			{
				syslog(LOG_ERR, "%s: calloc",__func__);
				exit(EXIT_FAILURE);
			}

			SPLAY_INSERT(ethertree, etherroot, entry);
		}

		if (entry->list_size >= entry->list_mem) {
			struct ethernetcode **tmp;

			/* We require more memory for this key word */
			entry->list_mem <<= 1;
			tmp = realloc(entry->list,
					entry->list_mem * sizeof(struct ethernetcode *));
			if (tmp == NULL)
			{
				syslog(LOG_ERR, "%s: realloc", __func__);
				exit(EXIT_FAILURE);
			}
			entry->list = tmp;
		}

		entry->list[entry->list_size++] = code;
	}

	return (0);
}

void ethernetcode_init(FILE *in_file){
	char currentLine[300];
	char currentChar;
	struct ethernetcode *currentCode;
	struct ethernetcode *codes;
	int counter = 0;
	int numOfLine = 0;
	uint32_t prefix;


	if (!in_file){
		printf("nmap-mac-prefixes File can't be found\n");
		syslog(LOG_ERR, "nmap-mac-prefixes file can't be found \n");
		exit(EXIT_FAILURE);
	}

	while(fgets( currentLine, 300, in_file ) != NULL){
		numOfLine++;
	}
	if(numOfLine > 0){
		codes = (struct ethernetcode *)malloc(numOfLine * sizeof(struct ethernetcode));
		currentCode = codes;
	}else{
		printf("nmap-mac-prefixes file cannot be parsed.");
		syslog(LOG_ERR, "nmap-mac-prefixes file can't be parsed \n");
		fclose(in_file);
		exit(EXIT_FAILURE);
	}
	rewind(in_file);
	do{
		memset (currentLine,'\0',300);
		if(fgets( currentLine, 300, in_file )==NULL)
		{

		}
		counter++;
	}while(currentLine[0] == '#');
	SPLAY_INIT(&etherroot);
		do{
		char routerID[20];
		char routerCompany[80];
		int i;
		for(i = 0; i < 20; i++)		{
			currentChar = currentLine[i];
			if(currentChar != ' '){
					routerID[i] = currentChar;
				}else{
					routerID[i] = '\0';
					break;
				}
		}
		int j;
		int companyNameStart = 0;
		int firstSpaceFound = 0;
		for(j = 0; j < 300; j++){
			currentChar = currentLine[j];
			if(currentChar >=65 && currentChar <= 90){
				currentChar = currentChar + 32;
		    }
			if(companyNameStart > 0){
			routerCompany[j-companyNameStart] = currentChar;
			}
			if(currentChar == ' ' && firstSpaceFound == 0){
				firstSpaceFound = 1;
				companyNameStart = j+1;
			}
			if(currentLine[j] == '\0' || currentLine[j] == '\n'){
				routerCompany[j-companyNameStart] = '\0';
				break;
			}

		}
		prefix = 0;
		sscanf(routerID, "%x", &prefix);
		currentCode->prefix = prefix;
		currentCode->vendor = routerCompany;
		ethernetcode_index(&etherroot, currentCode);
		++currentCode;
	}while (fgets( currentLine, 300, in_file ) != NULL);
	fclose(in_file);
}


/*
 * Returns the code that matches the best, 0 on error.
 */

static uint32_t
ethernetcode_find_best(struct etherindex **results, int size, int random)
{
	extern rand_t *honeyd_rand;
	int i, j, max = 0, count = 0;
	struct ethernetcode *code = NULL;

	if (!size)
		return (0);

	/* Reset the counters */
	for (i = 0; i < size; i++) {
		struct etherindex *ei = results[i];
		for (j = 0; j < ei->list_size; j++)
			ei->list[j]->count = 0;
	}

	for (i = 0; i < size; i++) {
		struct etherindex *ei = results[i];
		for (j = 0; j < ei->list_size; j++) {
			ei->list[j]->count++;
			if (ei->list[j]->count > max) {
				max = ei->list[j]->count;
				code = ei->list[j];
				count = 1;
			} else if (ei->list[j]->count == max && random) {
				/* Randomly select one of the best matches */
				count++;
				if (rand_uint8(honeyd_rand) % count == 0)
					code = ei->list[j];
			}
		}
	}

	return (code->prefix);
}

uint32_t
ethernetcode_find_prefix(char *vendor, int random) {
	struct etherindex *results[20];
	struct etherindex tmp, *entry;
	char line[1024], *p, *e;
	int pos = 0;

	strlcpy(line, vendor, sizeof(line));
	e = line;

	/* Walk through every single word and find the codes for it */
	while ((p = strsep(&e, " ")) != NULL && pos < 20) {
		int i;

		/* Change the string to lower case for the match */
		for (i = 0; i < strlen(p); i++)
			p[i] = tolower(p[i]);

		tmp.index_word = p;
		if ((entry = SPLAY_FIND(ethertree, &etherroot, &tmp)) == NULL)
			continue;

		results[pos++] = entry;
	}

	return (ethernetcode_find_best(results, pos, random));
}

struct addr *
ethernetcode_make_address(char *vendor)
{
	extern rand_t *honeyd_rand;
	uint32_t prefix = 0;
	u_char address[ETH_ADDR_LEN], *p;
	struct addr *ea;
	int i;

	/* Check if it is a regular mac address: xx:xx:xx:xx:xx:xx */
	p = address;
	for (i = 0; i < strlen(vendor) && p < address + ETH_ADDR_LEN; i += 3) {
		char hex[3];

		if (!isxdigit(vendor[i]) || !isxdigit(vendor[i+1]))
			break;

		hex[0] = vendor[i];
		hex[1] = vendor[i+1];
		hex[2] = '\0';

		*p++ = strtoul(hex, NULL, 16);

		if (i + 2 < strlen(vendor) && vendor[i + 2] != ':')
			break;
	}

	/* We could not parse the hex digits, so search for a vendor instead */
	if (p < address + ETH_ADDR_LEN) {
		if ((prefix = ethernetcode_find_prefix(vendor, 1)) == 0)
		{
			return (NULL);
		}

		/* We have a 24-bit prefix that is vendor dependant */
		address[2] = prefix & 0xff; prefix >>= 8;
		address[1] = prefix & 0xff; prefix >>= 8;
		address[0] = prefix & 0xff; prefix >>= 8;

		if (prefix != 0)
			return (NULL);

		for (i = 3; i < ETH_ADDR_LEN; i++)
			address[i] = rand_uint8(honeyd_rand);
	}

	if ((ea = calloc(1, sizeof(struct addr))) == NULL)
		return (NULL);

	addr_pack(ea, ADDR_TYPE_ETH, ETH_ADDR_BITS, address, ETH_ADDR_LEN);

	return (ea);
}

struct addr *
ethernetcode_clone(struct addr *src)
{
	extern rand_t *honeyd_rand;
	struct addr *ea;
	int i;

	if ((ea = calloc(1, sizeof(struct addr))) == NULL)
		return (NULL);

	memcpy(ea, src, sizeof(struct addr));

	/* Very low-level hack, might break when dnet changes */
	for (i = 3; i < ETH_ADDR_LEN; i++)
		ea->addr_data8[i] = rand_uint8(honeyd_rand);

	return (ea);
}

#define TEST(x, y) do { \
		if (ethernetcode_find_prefix(x, 0) != (y)) \
		{\
			syslog(LOG_ERR,"%s: %s does not match %.6x", __func__, x, y);\
			exit(EXIT_FAILURE);\
		}\
} while (0)

void
ethernetcode_test(void)
{
	TEST("cisco", 0x00000c);
	TEST("netkit solutions", 0x0003b8);
	TEST("juniper networks", 0x000585);
	TEST("cooperative linux virtual nic", 0x00ffd1);
	TEST("zzzzzzzz xxxxxxxx", 0x000000);

	fprintf(stderr, "\t%s: OK\n", __func__);
}

void
ethernet_test(void)
{
	FILE *in_file  = fopen("/usr/share/honeyd/nmap-mac-prefixes", "r"); // read only
	ethernetcode_init(in_file);
	ethernetcode_test();
}
