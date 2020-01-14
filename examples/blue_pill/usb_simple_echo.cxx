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

#include <core_cm3.hxx>

#include <stm32f103xb.hxx>

#include <usb_dev_simple.hxx>

#include <usb_echo.hxx>


using namespace stm32f103xb;
using namespace stm32f10_12357_xx;


UsbDevSimple    usb_dev;

uint8_t         recv_buf[UsbDevSimple::OUT_ENDPOINT_MAX_PACKET],
                send_buf[UsbDevSimple:: IN_ENDPOINT_MAX_PACKET];



int main()
{
    usb_echo::init();

    if (!usb_dev.init())
    {
        gpioc->bsrr = Gpio::Bsrr::BR13;  // turn on user LED by setting low
        while (true)    // hang
            asm("nop");
    }

    usb_echo::wait_configured();

    usb_echo::run(recv_buf                            ,
                  send_buf                            ,
                  UsbDevSimple::IN_ENDPOINT_MAX_PACKET,
                  UsbDevSimple::OUT_ENDPOINT          ,
                  UsbDevSimple::IN_ENDPOINT           );

}



