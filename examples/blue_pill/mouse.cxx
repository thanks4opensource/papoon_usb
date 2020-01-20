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

#include <sys_tick_timer.hxx>

#include <usb_dev_hid_mouse.hxx>

#include <usb_mcu_init.hxx>
#include <usb_randomtest.hxx>



using namespace stm32f103xb;
using namespace stm32f10_12357_xx;

arm::SysTickTimer   sys_tick_timer;
UsbDevHidMouse          usb_dev   ;

static const uint32_t   CPU_HZ         = 72000000   ,
                        TICKS_PER_MOVE = CPU_HZ / 24;  // 24 Hz




#ifdef USB_DEV_INTERRUPT_DRIVEN
extern "C" void USB_LP_CAN1_RX0_IRQHandler()
{
    usb_dev.interrupt_handler();
}
#endif



int main()
{
    usb_dev.serial_number_init();  // do before mcu_init() clock speed breaks

    usb_mcu_init ();
    usb_gpio_init();

    sys_tick_timer.init();

    gpioc->bsrr = Gpio::Bsrr::BS13;  // turn off user LED by setting high

#ifdef USB_DEV_INTERRUPT_DRIVEN
    arm::nvic->iser.set(arm::NvicIrqn::USB_LP_CAN1_RX0);
#endif

    if (!usb_dev.init())
    {
        gpioc->bsrr = Gpio::Bsrr::BR13;  // turn on user LED by setting low
        while (true) asm("nop");         // hang
    }

    gpioc->bsrr = Gpio::Bsrr::BR13;  // turn on user LED by setting low
    while (usb_dev.device_state() != UsbDev::DeviceState::CONFIGURED) {
#ifdef USB_DEV_INTERRUPT_DRIVEN
        asm("wfi");
#else
        usb_dev.poll();
#endif
    }
    gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED

    static const uint8_t    MAX_DIR  = 12,
                            MAX_STEP =  8,
                            X_NDX    =  1,
                            Y_NDX    =  2;

    static const int8_t
        X_DIRS    [MAX_DIR] = { 0, -1, -1, -1, -1,  0,  0,  1,  1,  1,  1,  0},
        Y_DIRS    [MAX_DIR] = { 1,  1,  0,  0, -1, -1, -1, -1,  0,  0,  1,  1};

    static uint8_t  hid_report[4] = { 0, 0, 0, 0};

    uint8_t     dir  = MAX_DIR  - 1,
                step = MAX_STEP - 1;

    while (true) {

        if (++step == MAX_STEP) {
            if (++dir == MAX_DIR) dir = 0;

            hid_report[X_NDX] = X_DIRS[dir];
            hid_report[Y_NDX] = Y_DIRS[dir];

            step = 0;
        }

        if (dir & 0x1)
            gpioc->bsrr = Gpio::Bsrr::BR13;  // set low  to turn on  user LED
        else
            gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED

        sys_tick_timer.begin32();

        while (!usb_dev.send(usb_dev.MOUSE_ENDPOINT_IN,
                             hid_report               ,
                             sizeof(hid_report)       ))
#ifdef USB_DEV_INTERRUPT_DRIVEN
            asm("wfi")    ;
#else
            usb_dev.poll();
#endif

        while (sys_tick_timer.elapsed32() < TICKS_PER_MOVE)
            asm("nop");
    }

}  // main()
