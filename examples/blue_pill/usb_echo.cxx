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

#include <core_cm3.hxx>

#include <stm32f103xb.hxx>

#include <bin_to_hex.hxx>
#include <sys_tick_timer.hxx>

#include <usb_dev.hxx>

#include <usb_mcu_init.hxx>




using namespace stm32f103xb;
using namespace stm32f10_12357_xx;


extern UsbDev   usb_dev;


arm::SysTickTimer   sys_tick_timer;

static const uint32_t   CPU_HZ           = 72000000,
                        LED_DELAY_TICK   = CPU_HZ / 50; // 0.020 seconds


#ifdef USB_DEV_INTERRUPT_DRIVEN
extern "C" void USB_LP_CAN1_RX0_IRQHandler()
{
    usb_dev.interrupt_handler();
}
#endif



namespace usb_echo {

void init()
{
    usb_dev.serial_number_init();  // do before mcu_init() clock speed breaks

    usb_mcu_init ();
    usb_gpio_init();

    sys_tick_timer.init();

    gpioc->bsrr = Gpio::Bsrr::BS13;  // turn off user LED by setting high

#ifdef USB_DEV_INTERRUPT_DRIVEN
    arm::nvic->iser.set(arm::NvicIrqn::USB_LP_CAN1_RX0);
#endif

}


void wait_configured()
{
    gpioc->bsrr = Gpio::Bsrr::BR13;  // turn on user LED by setting low

    while (usb_dev.device_state() != UsbDev::DeviceState::CONFIGURED)
#ifndef USB_DEV_INTERRUPT_DRIVEN
        usb_dev.poll();
#else
        asm("nop");
#endif

    gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED
}



void run(
      uint8_t       *recv_buf  ,
      uint8_t       *send_buf  ,
      uint8_t        send_max  ,
const uint8_t        recv_endpt,
const uint8_t        send_endpt)
{
    uint16_t    recv_len  = 0,
                msg_count = 0,
                send_len  = 0;

    while (true) {
        if (   !gpioc->odr.any(Gpio::Odr::ODR13)
            && sys_tick_timer.elapsed32() > LED_DELAY_TICK)
            gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED

#ifndef USB_DEV_INTERRUPT_DRIVEN
        usb_dev.poll();
#endif
        if (recv_len = usb_dev.recv(recv_endpt, recv_buf)) {
            gpioc->bsrr = Gpio::Bsrr::BR13;  // set low turn on user LED
            sys_tick_timer.begin32();

            uint8_t     recv_ndx  = 0,
                        sub_count = 0;
            while (recv_ndx < recv_len) {
                // 0123456...
                // cccc+ ....

                bitops::BinToHex::uint16(msg_count,
                                         reinterpret_cast<char*>(send_buf));

                if (sub_count++ == 0) send_buf[4] = ' ';
                else                  send_buf[4] = '+';

                send_buf[5] = ' ';

                send_len = 6;

                while (recv_ndx < recv_len && send_len < send_max - 1)
                    send_buf[send_len++] = recv_buf[recv_ndx++];
                send_buf[send_len++] = '\n';

                while (!usb_dev.send(send_endpt, send_buf, send_len))
#ifdef USB_DEV_INTERRUPT_DRIVEN
                    asm("wfi");
#else
                    usb_dev.poll();
#endif

            }

            if (send_len == send_max)
                // exactly IN endpoint size
                // have to send zero-length xfer to let host know is end
                while (!usb_dev.send(send_endpt, send_buf, 0))
#ifdef USB_DEV_INTERRUPT_DRIVEN
                    asm("wfi");
#else
                    usb_dev.poll();
#endif

            ++msg_count;
        }
    }
}

}  // namespace usb_echo
