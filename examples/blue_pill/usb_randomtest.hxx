// papoon_usb: "Not Insane" USB library for STM32F103xx MCUs
// Copyright (C) 2019 Mark R. Rubin
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


#ifndef USB_RANDOMTEST_HXX
#define USB_RANDOMTEST_HXX

#include <usb_dev.hxx>

namespace usb_randomtest {

void    init           (),
        wait_configured();

void run(const uint8_t  down_endpoint,
         const uint8_t    up_endpoint);

}  // namespace usb_randomtest

#endif   // ifndef USB_RANDOMTEST_HXX
