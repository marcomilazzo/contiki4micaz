
#include "sys/clock.h"
//#include "dev/clock-avr.h"
#include "sys/etimer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile clock_time_t count;

/*---------------------------------------------------------------------------*/
ISR(TIMER0_COMP_vect)
{
  ++count;
  if(etimer_pending()) {
    etimer_request_poll();
  }
}

/*---------------------------------------------------------------------------*/

void 
clock_init(void)
{
  /* disable interrupts*/
  cli();

  TIMSK &= ~( _BV(TOIE0) | _BV(OCIE0) );

  /** 
   * set Timer/Counter0 to be asynchronous 
   * from the CPU clock with a second external 
   * clock(32,768kHz) driving it.
   */
  ASSR |= _BV(AS0);

  /*
   * Set timer control register:
   * - prescale: 32 (CS00 and CS01)
   * - counter reset via comparison register (WGM01)
   */
  TCCR0 = _BV(CS00) | _BV(CS01) | _BV(WGM01);

  /* Set counter to zero */
  TCNT0 = 0;

  /*
   * 128 clock ticks per second.
   * 32,768 = 32 * 8 * 128
   */
  OCR0 = 8;

  /* Clear interrupt flag register */
  TIFR = 0x00;

  /**
   * Wait for TCN0UB, OCR0UB, and TCR0UB.
   *
   */
  while(ASSR & 0x07);

  /* Raise interrupt when value in OCR0 is reached. */
  TIMSK |= _BV(OCIE0);

  count = 0;

  /* enable all interrupts*/
  sei();

}

/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return count;
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of TODO
 */
void
clock_delay(unsigned int i)
{
  for (; i > 0; i--) {		/* Needs fixing XXX */
    unsigned j;
    for (j = 50; j > 0; j--)
      asm volatile("nop");
  }
}

/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of 1 / 128 sec = 7.8125 ms.
 *
 */
void
clock_wait(int i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
    // TODO
}

unsigned long
clock_seconds(void)
{
  return count / CLOCK_SECOND;
}
