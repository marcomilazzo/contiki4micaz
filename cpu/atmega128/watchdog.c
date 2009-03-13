#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "dev/watchdog.h"

static int stopped = 0;
/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
  stopped = 0;
  watchdog_stop();
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  cli();
  /* We setup the watchdog to reset the device after one second,
   * unless watchdog_periodic() is called.
   * See the page 54 of Atmega128 datasheet for more information.
   */
  stopped--;
  if(!stopped) {
    //WDTCR |= _BV(WDCE) | _BV(WDE);
    //WDTCR = 0x00;
    //WDTCR |= _BV(WDCE) | _BV(WDE) | _BV(WDP2) | _BV(WDP2);
    wdt_enable(WDTO_1S);
  }
  sei();

}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
  /* This function is called periodically to restart the watchdog
     timer. */
  if(!stopped) {
    wdt_reset();  // __asm__ __volatile__ ("wdr")
  }
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
  cli();
  //WDTCR |= _BV(WDCE) | _BV(WDE);
  //WDTCR = 0x00;
  wdt_disable();
  stopped++;
  sei();
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
 
}
/*---------------------------------------------------------------------------*/
