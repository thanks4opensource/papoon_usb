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


#ifndef USB_ECHO_HXX
#define USB_ECHO_HXX

namespace usb_echo {

void    init           (),
        wait_configured();

void    run(      uint8_t   *recv_buf  ,
                  uint8_t   *send_buf  ,
                  uint8_t    send_max  ,
            const uint8_t    recv_endpt,
            const uint8_t    send_endpt);

}  // namespace usb_echo

#endif   // ifndef USB_ECHO_HXX
