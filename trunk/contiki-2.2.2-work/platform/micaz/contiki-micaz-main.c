/*
 * Copyright (c) 2009, Kasun Hewage.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 *
 */

#include <avr/pgmspace.h>
#include <stdio.h>



#include "contiki.h"
#include "contiki-lib.h"

#include "dev/leds.h"
#include "dev/rs232.h"
#include "dev/serial.h"
#include "dev/cc2420.h"
#include "dev/watchdog.h"

#include "net/rime.h"
#include "net/mac/nullmac.h"
#include "net/mac/xmac.h"
#include "net/mac/lpp.h"

#ifndef RF_CHANNEL
#define RF_CHANNEL              26
#endif

PROCINIT(&etimer_process);


void
init_usart(void)
{
  /* Second rs232 port for debugging */
  rs232_init(RS232_PORT_0, USART_BAUD_28800,
             USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);

  /* Redirect stdout to second port */
  rs232_redirect_stdout(RS232_PORT_0);


}

static void
set_rime_addr(void)
{

  rimeaddr_t rimeaddr;
  rimeaddr.u8[0] = 44;
  rimeaddr.u8[1] = 44;
  rimeaddr_set_node_addr(&rimeaddr);
}


int
main(void)
{

  leds_init();

  leds_on(LEDS_ALL);

  /* Initialize USART */
  init_usart();

  /* Clock */
  clock_init();

  random_init(0);
  rtimer_init();
  /* Process subsystem */
  process_init();

  /* Register initial processes */
  procinit_init();
  ctimer_init();

  cc2420_init();
  cc2420_set_channel(RF_CHANNEL);

#if WITH_NULLMAC
  rime_init(nullmac_init(&cc2420_driver));
#else
  //rime_init(xmac_init(&cc2420_driver));
  rime_init(lpp_init(&cc2420_driver));
#endif
  set_rime_addr();

  printf_P(PSTR("\n********BOOTING CONTIKI*********\n"));

  printf_P(PSTR(CONTIKI_VERSION_STRING " started. \n"));

  leds_off(LEDS_ALL);

  /* Autostart processes */
  autostart_start(autostart_processes);

  //watchdog_start();
  //watchdog_stop();

  /* Main scheduler loop */
  do {

    /* Reset watchdog. */
    //watchdog_periodic();

    process_run();

  }while(1);

  return 0;
}
