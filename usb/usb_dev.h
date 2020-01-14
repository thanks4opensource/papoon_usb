// papoon_usb: "Not Insane" USB library for STM32F103xx MCUs
// Copyright (C) 2019,2020 Mark R. Rubin
//
// This file is part of papoon_usb.
//
// The regbits_stm program is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// The regbits_stm program is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// (LICENSE.txt) along with the regbits_stm program.  If not, see
// <https://www.gnu.org/licenses/gpl.html>


#ifndef USB_DEV_H
#define USB_DEV_H

#include <stdint.h>

enum {
    CONSTRUCTED = 0,
    INITIALIZED    ,
    RESET          ,
    ADDRESSED      ,
    CONFIGURED
} DeviceState;


/* Optional use by client application.
   Will copy/format ST "Unique Device ID" value into USB descriptor,
   Otherwise value will be  default string "000..."
   Hardware bug: must be done while main CPU clock at default 8 MHz
     before mandatory increase to 48 or 72 MHz for USB peripheral.
*/
void        usb_dev_serial_number_init();

int         usb_dev_init();

unsigned    usb_dev_device_state();

void        usb_dev_interrupt_handler();

#ifndef USB_DEV_INTERRUPT_DRIVEN
uint32_t    usb_dev_poll();

uint32_t    usb_dev_poll_recv_ready(const uint8_t   endpoint),
            usb_dev_poll_send_ready(const uint8_t   endpoint);
#endif

#ifdef USB_DEV_ENDPOINT_CALLBACKS
void        usb_dev_register_recv_callback(void      (*callback)(const uint8_t,
                                                                 void*        ),
                                           uint8_t   endpoint,  /* not checked */
                                           void     *user_data                 ),
            usb_dev_register_send_callback(void      (*callback)(const uint8_t,
                                                                 void*        ),
                                           uint8_t   endpoint,  /* not checked */
                                           void     *user_data                 );
#endif

uint16_t    usb_dev_endpoint_recv_bufsize(const uint8_t endpoint),
            usb_dev_endpoint_send_bufsize(const uint8_t endpoint);

uint16_t    usb_dev_recv_readys(),
            usb_dev_send_readys();

// no checking of params -- caller must guarantee valid
//   endpoint_number and buffer
uint16_t    usb_dev_recv(const uint8_t          endpoint_number,
                               uint8_t* const   buffer         );

// no checking of params -- caller must guarantee valid
//   endpoint_number, data, and length
int          usb_dev_send(const uint8_t         endpoint_number,
                          const uint8_t* const  data           ,
                          const uint16_t        length         );

#endif  // ifndef USB_DEV_H
