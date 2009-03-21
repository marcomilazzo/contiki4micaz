#ifndef __MTS300_H__
#define __MTS300_H__

#include <avr/io.h>
#include "contiki-conf.h"

#define SOUNDER_PORT PORTC
#define SOUNDER_MASK _BV(2)
#define SOUNDER_DDR DDRC

/* MTS300CA and MTS310CA, the light sensor power is controlled 
 * by setting signal INT1(PORTE pin 5).
 * Both light and thermistor use the same ADC channel.
 */
#define LIGHT_PORT_DDR DDRE
#define LIGHT_PORT PORTE
#define LIGHT_PIN_MASK _BV(5)
#define LIGHT_ADC_CHANNEL 1

/* MTS300CA and MTS310CA, the thermistor power is controlled 
 * by setting signal INT2(PORTE pin 6).
 * Both light and thermistor use the same ADC channel.
 */
#define TEMP_PORT_DDR DDRE
#define TEMP_PORT PORTE
#define TEMP_PIN_MASK _BV(6)
#define TEMP_ADC_CHANNEL 1

/* Power is controlled to the accelerometer by setting signal
 * PW4(PORTC pin 4), and the analog data is sampled on ADC3 and ADC4.
 */
#define ACCEL_PORT_DDR DDRC
#define ACCEL_PORT PORTC
#define ACCEL_PIN_MASK _BV(4)
#define ACCELX_ADC_CHANNEL 3
#define ACCELY_ADC_CHANNEL 4

/* Power is controlled to the magnetometer by setting signal
 * PW5(PORTC pin 5), and the analog data is sampled on ADC5 and ADC6.
 */
#define MAGNET_PORT_DDR DDRC
#define MAGNET_PORT PORTC
#define MAGNET_PIN_MASK _BV(5)
#define MAGNETX_ADC_CHANNEL 5
#define MAGNETY_ADC_CHANNEL 6


#define MIC_PORT_DDR DDRC
#define MIC_PORT PORTC
#define MIC_PIN_MASK _BV(3)
#define MIC_ADC_CHANNEL 2

void sounder_on();
void sounder_off();

uint16_t get_light();
uint16_t get_temp();

uint16_t get_accx();
uint16_t get_accy();

uint16_t get_magx();
uint16_t get_magy();

uint16_t get_mic();

void mts300_init();

#endif /* __MTS300_H__ */



