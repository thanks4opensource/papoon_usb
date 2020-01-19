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

#include <bin_to_hex.hxx>
#include <sys_tick_timer.hxx>

#include <usb_dev_max_endpts.hxx>

#include <usb_echo.hxx>


using namespace stm32f103xb;
using namespace stm32f10_12357_xx;



static const uint32_t   CPU_HZ           = 72000000,
                        LED_DELAY_TICK   = CPU_HZ / 50; // 0.020 seconds

extern arm::SysTickTimer    sys_tick_timer;


UsbDevMaxEndpts     usb_dev;


uint8_t             recv_buf[UsbDevMaxEndpts::OUT_ENDPOINTS_MAX_PACKET],
                    send_buf[UsbDevMaxEndpts:: IN_ENDPOINTS_MAX_PACKET];



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

    uint16_t    recv_len  = 0,
                send_len  = 0,
                msg_count = 0;

    while (true) {
        if (   !gpioc->odr.any(Gpio::Odr::ODR13)
            && sys_tick_timer.elapsed32() > LED_DELAY_TICK)
            gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED

#ifndef USB_DEV_INTERRUPT_DRIVEN
        usb_dev.poll();
#endif

        for (uint8_t      endpt_ndx =  0                                    ;
                          endpt_ndx < UsbDevMaxEndpts::NUM_IN_OUT_ENDPOINTS ;
                        ++endpt_ndx                                          ) {

            uint8_t     recv_len,
                        endpt_addr =   UsbDevMaxEndpts
                                     ::ENDPOINT_ADDRESSES[endpt_ndx];

            if (recv_len = usb_dev.recv(endpt_addr, recv_buf)) {
                gpioc->bsrr = Gpio::Bsrr::BR13;  // set low turn on user LED
                sys_tick_timer.begin32();

                uint8_t     recv_ndx  = 0,
                            sub_count = 0;
                while (recv_ndx < recv_len) {
                    // 012345678...
                    // a ccccl ....

                    bitops::BinToHex::uint4(endpt_addr,
                                            reinterpret_cast<char*>(send_buf));
                    send_buf[1] = ' ';

                    bitops::BinToHex::uint16(msg_count,
                                             reinterpret_cast<char*>
                                             (send_buf + 2));

                                                      // already incremented
                    send_buf[6] = sub_count++ ? 'a' - 2 + sub_count : ' ';
                    send_buf[7] =                                     ' ';

                    send_len = 8;

                    while (  recv_ndx < recv_len
                           &&   send_len
                              < UsbDevMaxEndpts::IN_ENDPOINTS_MAX_PACKET - 1)
                        send_buf[send_len++] = recv_buf[recv_ndx++];
                    send_buf[send_len++] = '\n';

                    while (!usb_dev.send(endpt_addr, send_buf, send_len))
    #ifdef USB_DEV_INTERRUPT_DRIVEN
                        asm("wfi");
    #else
                        usb_dev.poll();
    #endif
                }

                if (send_len == UsbDevMaxEndpts::IN_ENDPOINTS_MAX_PACKET)
                    // exactly IN endpoint size
                    // have to send zero-length xfer to let host know is end
                    while (!usb_dev.send(endpt_addr, send_buf, 0))
    #ifdef USB_DEV_INTERRUPT_DRIVEN
                        asm("wfi");
    #else
                        usb_dev.poll();
    #endif

                ++msg_count;
            }
        }
    }
}
