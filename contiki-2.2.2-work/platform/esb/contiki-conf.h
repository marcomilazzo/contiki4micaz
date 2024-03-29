#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define RIMEBUF_CONF_ATTRS_INLINE 1
#define QUEUEBUF_CONF_NUM 1
#define QUEUEBUF_CONF_REF_NUM 1
#define ROUTE_CONF_ENTRIES 4

#define SERIAL_CONF_BUFSIZE 60

#define TIMESYNCH_CONF_ENABLED 0
#define PROFILE_CONF_ON 0
#define ENERGEST_CONF_ON 1

#define HAVE_STDINT_H
#include "msp430def.h"

#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 0

/* CPU target speed in Hz */
#define F_CPU 2457600uL

/* Our clock resolution, this is the same as Unix HZ. Must be a power
   of two (see clock.c for details). */
#define CLOCK_CONF_SECOND 64

#define IRQ_PORT1_VECTOR 1

#define IRQ_PORT1 0x01
#define IRQ_PORT2 0x02
#define IRQ_ADC   0x03

#define NODE_ID_EEPROM_OFFSET     0x0010 /* - 0x0014 */
#define CFS_EEPROM_CONF_OFFSET    0x0040

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1

#define CC_CONF_INLINE inline

#define CC_CONF_VA_ARGS                1

#define CCIF
#define CLIF

typedef unsigned short clock_time_t;

void clock_wait(int ms10);

void clock_set_seconds(unsigned long s);
unsigned long clock_seconds(void);

#define LOG_CONF_ENABLED 0

/**
 * The statistics data type.
 *
 * This datatype determines how high the statistics counters are able
 * to count.
 */
typedef unsigned short uip_stats_t;

#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_IP_FORWARD      1
#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#define UIP_CONF_BUFFER_SIZE     110
#define UIP_CONF_RECEIVE_WINDOW  (UIP_CONF_BUFFER_SIZE - 40)
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       6
#define UIP_CONF_FWCACHE_SIZE    1
#define UIP_CONF_BROADCAST       1
#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0
#define UIP_CONF_RESOLV_ENTRIES  1

#define UIP_CONF_TCP_SPLIT       0

#define LOADER_CONF_ARCH "loader/loader-arch.h"

#define ELFLOADER_CONF_DATAMEMORY_SIZE 100
#define ELFLOADER_CONF_TEXTMEMORY_SIZE 0x1000

/* LEDs ports. */
#define LEDS_PxDIR P2DIR
#define LEDS_PxOUT P2OUT
#define LEDS_CONF_RED    0x01
#define LEDS_CONF_GREEN  0x02
#define LEDS_CONF_YELLOW 0x04

#endif /* __CONTIKI_CONF_H__ */
