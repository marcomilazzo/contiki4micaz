/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file rndis_task.c *********************************************************
 *
 * \brief
 *      Manages the RNDIS Dataclass for the USB Device
 *
 * \addtogroup usbstick
 *
 * \author 
 *        Colin O'Flynn <coflynn@newae.com>
 *
 ******************************************************************************/
/* Copyright (c) 2008  ATMEL Corporation
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
/**
 \addtogroup RNDIS
 @{
 */

//_____  I N C L U D E S ___________________________________________________


#include "contiki.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"
#include "rndis/rndis_task.h"
#include "rndis/rndis_protocol.h"
#include "uip.h"
#include "sicslow_ethernet.h"
#include <stdio.h>

#include <avr/pgmspace.h>
#include <util/delay.h>

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define PRINTF printf
#define PRINTF_P printf_P

//_____ M A C R O S ________________________________________________________





//_____ D E F I N I T I O N S ______________________________________________


#define IAD_TIMEOUT_DETACH 400
#define IAD_TIMEOUT_ATTACH 800

#define RNDIS_TIMEOUT_DETACH 900
#define RNDIS_TIMEOUT_ATTACH 1000

#define PBUF ((rndis_data_packet_t *) data_buffer)

//_____ D E C L A R A T I O N S ____________________________________________


//! Timers for LEDs
uint8_t led1_timer, led2_timer;

//! Temp data buffer when adding RNDIS headers
uint8_t data_buffer[64];

//! Usb is busy with RNDIS
char usb_busy = 0;


static struct etimer et;
static struct timer flood_timer;
static uint16_t iad_fail_timeout, rndis_fail_timeout;	

static uint8_t doInit = 1;

extern uint8_t fingerPresent;

PROCESS(rndis_process, "RNDIS process");

/**
 * \brief RNDIS Process
 *
 *   This is the link between USB and the "good stuff". In this routine data
 *   is received and processed by RNDIS
 */
PROCESS_THREAD(rndis_process, ev, data_proc)
{

	PROCESS_BEGIN();
	uint8_t bytecounter, headercounter;
	uint16_t i, dataoffset;
	clock_time_t timediff;
	clock_time_t thetime;

	while(1) {

		// turn off LED's if necessary
	    if (led1_timer) led1_timer--;
	    else            Led1_off();
	    if (led2_timer) led2_timer--;
	    else            Led2_off();
		
		/* Device is Enumerated but RNDIS not loading. We might
		   have a system that does not support IAD (winXP). If so
		   count the timeout then switch to just network interface. */
		if (usb_mode == rndis_debug) {
			//If we have timed out, detach
			if (iad_fail_timeout == IAD_TIMEOUT_DETACH) {
			
				//Failed - BUT we are using "reverse logic", hence we force device
				//into this mode. This is used to allow Windows Vista have time to
				//install the drivers
				if (fingerPresent && (rndis_state != rndis_data_initialized) && Is_device_enumerated() ) {
					iad_fail_timeout = 0;
				} else {
						stdout = NULL;
						Usb_detach();
						doInit = 1; //Also mark system as needing intilizing
				}
				
			//Then wait a few before re-attaching
			} else if (iad_fail_timeout == IAD_TIMEOUT_ATTACH) {
			
			    if (fingerPresent) {
					usb_mode = mass_storage;
				} else {
					usb_mode = rndis_only;
				}
				Usb_attach();
			}
	
			//Increment timeout when device is not initializing, OR we have already detached,
			//OR the user had their finger on the device, indicating a reverse of logic
			if ( ( (rndis_state != rndis_data_initialized) && Is_device_enumerated() ) ||
			  (iad_fail_timeout > IAD_TIMEOUT_DETACH) || 
			   (fingerPresent) ) {
				iad_fail_timeout++;
			} else {	
			iad_fail_timeout = 0;
			}
		} //usb_mode == rndis_debug


	     /* Device is Enumerated but RNDIS STIL not loading. We just
		    have RNDIS interface, so obviously no drivers on target.
			Just go ahead and mount ourselves as mass storage... */
		if (usb_mode == rndis_only) {
			//If we have timed out, detach
			if (rndis_fail_timeout == RNDIS_TIMEOUT_DETACH) {
				Usb_detach();
			//Then wait a few before re-attaching
			} else if (rndis_fail_timeout == RNDIS_TIMEOUT_ATTACH) {
				usb_mode = mass_storage;
				Usb_attach();
			}
	
			//Increment timeout when device is not initializing, OR we are already
			//counting to detach
			if ( ( (rndis_state != rndis_data_initialized)) ||
			  (rndis_fail_timeout > RNDIS_TIMEOUT_DETACH) ) {
				rndis_fail_timeout++;
			} else {	
			rndis_fail_timeout = 0;
			}
		}//usb_mode == rnids_only


	    if(rndis_state == rndis_data_initialized) //Enumeration processs OK ?
	    {
			if (doInit) {
				//start flood timer
				timer_set(&flood_timer, CLOCK_SECOND / 5);
				
				mac_ethernetSetup();
				doInit = 0;
			}

			//Connected!
			Led0_on();

			Usb_select_endpoint(RX_EP);

			//If we have data and a free buffer
			if(Is_usb_receive_out() && (uip_len == 0)) {
			
			    //TODO: Fix this some better way
				//If you need a delay in RNDIS to slow down super-fast sending, insert it here
				//Also mark the USB as "in use"
								
				//This is done as "flood control" by only allowing one IP packet per time limit
				thetime = clock_time();
				
				timediff = thetime - flood_timer.start;
				
				//Oops, timer wrapped! Just ignore it for now
				if (thetime < flood_timer.start) {
					timediff = flood_timer.interval;
				}
				
								
				//If timer not yet expired
				//if (timediff < flood_timer.interval) {
				if (!timer_expired(&flood_timer)) {
					//Wait until timer expiers
					usb_busy = 1;
					etimer_set(&et, flood_timer.interval - timediff);
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));     

					//Reselect endpoint in case we lost it
					Usb_select_endpoint(RX_EP);          
					usb_busy = 0;
				}				
				

				//Restart flood timer
				timer_restart(&flood_timer);

				//Read how much (endpoint only stores up to 64 bytes anyway)
				bytecounter = Usb_byte_counter_8();
 
				//Try and read the header in
				headercounter = sizeof(rndis_data_packet_t);

				uint8_t fail = 0;

				//Hmm.. what's going on here
				if (bytecounter < headercounter) {
					Usb_ack_receive_out();
					fail = 1;
				}

				i = 0;
				while (headercounter) {
					data_buffer[i] = Usb_read_byte();
					bytecounter--;
					headercounter--;
					i++;
				}

				//This is no good. Probably lost syncronization... just drop it for now
				if(PBUF->MessageType != REMOTE_NDIS_PACKET_MSG) {
					Usb_ack_receive_out();
					fail = 1;
				}

				//802.3 does not have OOB data, and we don't care about per-packet data
				//so that just leave regular packet data...
				if (PBUF->DataLength && (fail == 0)) {			
				
					//Looks like we've got a live one
					rx_start_led();			


					//Get offset
					dataoffset = PBUF->DataOffset;

					//Make it offset from start of message, not DataOffset field
					dataoffset += (sizeof(rndis_MessageType_t) + sizeof(rndis_MessageLength_t));

					//Subtract what we already took
					dataoffset -= sizeof(rndis_data_packet_t);

					//Read to the start of data
					while(dataoffset) {
						Usb_read_byte();
						dataoffset--;
						bytecounter--;

						//If endpoint is done
						if (bytecounter == 0) {	
				
							Usb_ack_receive_out();		
					

							//Wait for new data
							while (!Is_usb_receive_out());
					
					
							bytecounter = Usb_byte_counter_8();
						}

					}
			
					//Read the data itself in
					uint8_t * uipdata = uip_buf;
					uint16_t datalen = PBUF->DataLength;
			
					while(datalen) {
						*uipdata++ = Usb_read_byte();
						datalen--;
						bytecounter--;

						//If endpoint is done
						if (bytecounter == 0) {
							//Might be everything we need!
							if (datalen) {
								Usb_ack_receive_out();
								//Wait for new data
								while (!Is_usb_receive_out());
								bytecounter = Usb_byte_counter_8();
							}
						}

					}

					//Ack final data packet
					Usb_ack_receive_out();					

					//Send data over RF or to local stack
					uip_len = PBUF->DataLength; //uip_len includes LLH_LEN
					mac_ethernetToLowpan(uip_buf);
					

				} //if (PBUF->DataLength) 


		}  //if(Is_usb_receive_out() && (uip_len == 0))
        
	    } // if (rndis_data_intialized)

		if ((usb_mode == rndis_only) || (usb_mode == rndis_debug)) {
			etimer_set(&et, CLOCK_SECOND/80);
		} else {
			etimer_set(&et, CLOCK_SECOND);
		}

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	} // while(1)

	PROCESS_END();
}

/**
 \brief Send data over RNDIS interface, data is in uipbuf and length is uiplen
 */
uint8_t rndis_send(uint8_t * senddata, uint16_t sendlen, uint8_t led)
{


	uint16_t i;

	//Setup Header
	PBUF->MessageType = REMOTE_NDIS_PACKET_MSG;
	PBUF->DataOffset = sizeof(rndis_data_packet_t) - sizeof(rndis_MessageType_t) - sizeof(rndis_MessageLength_t);
	PBUF->DataLength = sendlen;
	PBUF->OOBDataLength = 0;
	PBUF->OOBDataOffset = 0;
	PBUF->NumOOBDataElements = 0;
	PBUF->PerPacketInfoOffset = 0;
	PBUF->PerPacketInfoLength = 0;
	PBUF->DeviceVcHandle = 0;
	PBUF->Reserved = 0;
	PBUF->MessageLength = sizeof(rndis_data_packet_t) + PBUF->DataLength;

	//Send Data
	Usb_select_endpoint(TX_EP);
	Usb_send_in();

	//Wait for ready
	while(!Is_usb_write_enabled());

	//Setup first part of transfer...
	for(i = 0; i < sizeof(rndis_data_packet_t); i++) {
		Usb_write_byte(data_buffer[i]);
	}
	
	//Send packet
	while(sendlen) {
		Usb_write_byte(*senddata);
		senddata++;
		sendlen--;

		//If endpoint is full, send data in
		//And then wait for data to transfer
		if (!Is_usb_write_enabled()) {
			Usb_send_in();

			while(!Is_usb_write_enabled());
		}

		if (led) {
			tx_end_led();
		}
	}

	Usb_send_in();

	return 1;
}

/**
    @brief This will enable the RX_START LED for a period
*/
void rx_start_led(void)
{
    Led1_on();
    led1_timer = 10;
}

/**
    @brief This will enable the TRX_END LED for a period
*/
void tx_end_led(void)
{
    Led2_on();
    led2_timer = 10;
}
/** @}  */
