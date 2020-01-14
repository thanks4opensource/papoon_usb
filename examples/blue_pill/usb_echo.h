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


#ifndef USB_ECHO_H
#define USB_ECHO_H

void    usb_echo_init            (),
        usb_echo_wait_configured();

void    usb_echo_run(      uint8_t  *recv_buf  ,
                           uint8_t  *send_buf  ,
                           uint8_t   send_max  ,
                     const uint8_t   recv_endpt,
                     const uint8_t   send_endpt);

#endif   // ifndef USB_ECHO_H
