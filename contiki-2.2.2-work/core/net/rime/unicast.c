
/**
 * \addtogroup rimeuc
 * @{
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: unicast.c,v 1.1 2008/06/26 11:19:22 adamdunkels Exp $
 */

/**
 * \file
 *         Single-hop unicast
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime.h"
#include "net/rime/unicast.h"
#include <string.h>

static const struct rimebuf_attrlist attributes[] =
  {
    UNICAST_ATTRIBUTES
    RIMEBUF_ATTR_LAST
  };

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
recv_from_broadcast(struct broadcast_conn *broadcast, rimeaddr_t *from)
{
  struct unicast_conn *c = (struct unicast_conn *)broadcast;

  PRINTF("%d.%d: uc: recv_from_broadcast, receiver %d.%d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 rimebuf_addr(RIMEBUF_ADDR_RECEIVER)->u8[0],
	 rimebuf_addr(RIMEBUF_ADDR_RECEIVER)->u8[1]);
  if(rimeaddr_cmp(rimebuf_addr(RIMEBUF_ADDR_RECEIVER), &rimeaddr_node_addr)) {
    c->u->recv(c, from);
  }
}
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks uc = {recv_from_broadcast};
/*---------------------------------------------------------------------------*/
void
unicast_open(struct unicast_conn *c, uint16_t channel,
	     const struct unicast_callbacks *u)
{
  broadcast_open(&c->c, channel, &uc);
  c->u = u;
  channel_set_attributes(channel, attributes);
}
/*---------------------------------------------------------------------------*/
void
unicast_close(struct unicast_conn *c)
{
  broadcast_close(&c->c);
}
/*---------------------------------------------------------------------------*/
int
unicast_send(struct unicast_conn *c, const rimeaddr_t *receiver)
{
  PRINTF("%d.%d: unicast_send to %d.%d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 receiver->u8[0], receiver->u8[1]);
  rimebuf_set_addr(RIMEBUF_ADDR_RECEIVER, receiver);
  return broadcast_send(&c->c);
}
/*---------------------------------------------------------------------------*/
/** @} */
