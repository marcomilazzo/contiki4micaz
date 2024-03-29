/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * $Id: contiki-main.c,v 1.9 2008/07/09 20:58:25 adamdunkels Exp $
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

#include "contiki.h"

#include "dev/serial.h"

#include "net/uip.h"

#include "dev/button-sensor.h"
#include "dev/pir-sensor.h"
#include "dev/vib-sensor.h"

PROCINIT(&etimer_process, &tcpip_process, &serial_process);

SENSORS(&pir_sensor, &vib_sensor, &button_sensor);

/*---------------------------------------------------------------------------*/
int
main(void)
{
  printf("Starting Contiki\n");
  process_init();

  procinit_init();
  
  autostart_start(autostart_processes);

  /* Make standard output unbuffered. */
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
  
  while(1) {
    fd_set fds;
    int n;
    struct timeval tv;
    
    n = process_run();

    
    tv.tv_sec = 0;
    tv.tv_usec = 1;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(1, &fds, NULL, NULL, &tv);

    if(FD_ISSET(STDIN_FILENO, &fds)) {
      char c;
      if(read(STDIN_FILENO, &c, 1) > 0) {
	serial_input_byte(c);
      }
    }
    
    etimer_request_poll();
  }
  
  return 0;
}
/*---------------------------------------------------------------------------*/
void
log_message(char *m1, char *m2)
{
  printf("%s%s\n", m1, m2);
}
/*---------------------------------------------------------------------------*/
void
uip_log(char *m)
{
  printf("%s\n", m);
}
/*---------------------------------------------------------------------------*/
