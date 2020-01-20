// papoon_usb: "Not Insane" USB library for STM32F103xx MCUs
// Copyright (C) 2019,2020 Mark R. Rubin
//
// This file is part of papoon_usb.
//
// The papoon_usb program is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// The papoon_usb program is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// (LICENSE.txt) along with the papoon_usb program.  If not, see
// <https://www.gnu.org/licenses/gpl.html>


#include <stdint.h>

#include <usb_dev_cdc_acm.h>

#include <usb_echo.h>



uint8_t     recv_buf[CDC_OUT_DATA_SIZE],
            send_buf[CDC_IN_DATA_SIZE ];



int main()
{
    usb_echo_init();

    if (!usb_dev_init())
        while (1)   // hang
            asm("nop");

    usb_echo_wait_configured();

    usb_echo_run(recv_buf        ,
                 send_buf        ,
                 CDC_IN_DATA_SIZE,
                 CDC_ENDPOINT_OUT,
                 CDC_ENDPOINT_IN );
}
