#include "contiki.h"
#include "net/rime.h"
#include "dev/leds.h"
#include <stdio.h>
#include <avr/pgmspace.h>

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, rimeaddr_t *from)
{

  printf_P(PSTR("unicast message received from %d.%d\n"),from->u8[0], from->u8[1]);
  leds_on(LEDS_RED);
  clock_wait(1);
  leds_off(LEDS_RED);

}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  rimeaddr_t rimeaddr;

  PROCESS_EXITHANDLER(unicast_close(&uc);)

  PROCESS_BEGIN();


  rimeaddr.u8[0] = 44;
  rimeaddr.u8[1] = 44;
  rimeaddr_set_node_addr(&rimeaddr);

  unicast_open(&uc, 128, &unicast_callbacks);

  while(1) {
    static struct etimer et;
    rimeaddr_t addr;

    etimer_set(&et, CLOCK_SECOND * 2);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    rimebuf_copyfrom("Hello", 5);
    addr.u8[0] = 22;
    addr.u8[1] = 22;
    unicast_send(&uc, &addr);

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
