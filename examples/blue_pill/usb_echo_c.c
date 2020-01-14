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


#include <stdint.h>

#include <stm32f103xb.h>
#include <core_cm3.h>

#include <usb_mcu_init.h>

#include <usb_dev_cdc_acm.h>



#ifdef USB_DEV_INTERRUPT_DRIVEN
void USB_LP_CAN1_RX0_IRQHandler()
{
    usb_dev_interrupt_handler();
}
#endif




void usb_echo_init()
{
    usb_dev_serial_number_init();  // do before mcu_init() clock speed breaks

    usb_mcu_init();

#ifdef USB_DEV_INTERRUPT_DRIVEN
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
#endif

}


void usb_echo_wait_configured()
{
    while (usb_dev_device_state() != CONFIGURED)
#ifndef USB_DEV_INTERRUPT_DRIVEN
        usb_dev_poll();
#else
        asm("nop");
#endif
}



static char hex(
const uint8_t   nibble)
{
    return '0' + nibble + (nibble > 9 ? 'a' - ':' : 0);
}

static void byte(
const uint8_t    byte     ,
const uint8_t    position ,
  char      *hex_chars)
{
    hex_chars[position    ] = hex(byte >> 4 );
    hex_chars[position + 1] = hex(byte  & 0x0f);
}

static char* bin_to_hex_uint16(
const uint16_t   bin,
      char      *hex)
{
    byte(bin >>    8, 0, hex);
    byte(bin  & 0xff, 2, hex);
    return hex;
}

static const char* bin_to_hex_uint8(
const uint8_t    bin,
      char      *hex)
{
    byte(bin, 0, hex);
    return hex;
}



void usb_echo_run(
      uint8_t       *recv_buf  ,
      uint8_t       *send_buf  ,
      uint8_t        send_max  ,
const uint8_t        recv_endpt,
const uint8_t        send_endpt)
{
    uint16_t    recv_len  = 0,
                msg_count = 0,
                send_len  = 0;

    while (1) {
#ifndef USB_DEV_INTERRUPT_DRIVEN
        usb_dev_poll();
#endif
        if (recv_len = usb_dev_recv(recv_endpt, recv_buf)) {
            uint8_t     recv_ndx  = 0,

                        sub_count = 0;
            while (recv_ndx < recv_len) {
                bin_to_hex_uint16(msg_count, (char*)send_buf);
                send_buf[4] = ' ';
                send_len    = 5  ;

                bin_to_hex_uint8(sub_count++, (char*)(send_buf + 5));
                send_buf[7] = ' ';
                send_len    = 8  ;

                while (recv_ndx < recv_len && send_len < send_max - 1)
                    send_buf[send_len++] = recv_buf[recv_ndx++];
                send_buf[send_len++] = '\n';

                while (!usb_dev_send(send_endpt, send_buf, send_len))
#ifdef USB_DEV_INTERRUPT_DRIVEN
                    asm("wfi");
#else
                    usb_dev_poll();
#endif

                if (send_len == 64)
                    // exactly IN endpoint size
                    // have to send zero-length xfer to let host know is end
                    while (!usb_dev_send(send_endpt, send_buf, 0))
#ifdef USB_DEV_INTERRUPT_DRIVEN
                        asm("wfi");
#else
                        usb_dev_poll();
#endif

            }
            ++msg_count;
        }
    }
}
