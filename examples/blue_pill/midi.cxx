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

#include <usb_dev_midi.hxx>

#include <usb_mcu_init.hxx>
#include <usb_randomtest.hxx>



using namespace stm32f103xb;
using namespace stm32f10_12357_xx;

arm::SysTickTimer   sys_tick_timer;
UsbDevMidi          usb_dev   ;

static const uint32_t   CPU_HZ             = 72000000       ;
static const uint64_t   MIDI_NOTE_ON_TIME  = CPU_HZ     /  4,   // 0.25  seconds
                        MIDI_NOTE_OFF_TIME = CPU_HZ * 3 /  4;   // 0.75  seconds

static const uint32_t   MIDI_CABLE    = 0   ,
                        MIDI_CHANNEL  = 0   ,
                        MIDI_NOTE_ON  = 0x90,
                        MIDI_NOTE_OFF = 0x80,
                        MIDI_VELOCITY = 0x40;

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


    static const uint8_t        MIDI_NOTES[] = {60, 62, 64, 65, 67, 69, 71, 72},
                            NUM_MIDI_NOTES   = sizeof(MIDI_NOTES)              ;

    uint8_t                             midi_note_ndx = 0;
    volatile UsbDevMidi::EventPacket    packet           ;

    while (true) {
        packet.populate(0,
                        UsbDevMidi::EventPacket::NOTE_ON,
                        MIDI_NOTE_ON | MIDI_CHANNEL    ,
                        MIDI_NOTES[midi_note_ndx]      ,
                        MIDI_VELOCITY                   );

        while (!usb_dev.send(usb_dev.BULK_IN_ENDPOINT           ,
                             static_cast<const uint8_t*>(packet),
                             sizeof                     (packet)))
#ifdef USB_DEV_INTERRUPT_DRIVEN
            asm("wfi")    ;
#else
            usb_dev.poll();
#endif

        gpioc->bsrr = Gpio::Bsrr::BR13           ;  // low to turn on user LED
        sys_tick_timer.delay64(MIDI_NOTE_ON_TIME);

        sys_tick_timer.begin64();

        packet.populate(0,
                        UsbDevMidi::EventPacket::NOTE_OFF,
                        MIDI_NOTE_OFF | MIDI_CHANNEL     ,
                        MIDI_NOTES[midi_note_ndx]        ,
                        MIDI_VELOCITY                    );

        while (!usb_dev.send(usb_dev.BULK_IN_ENDPOINT           ,
                             static_cast<const uint8_t*>(packet),
                             sizeof                     (packet))) {
#ifdef USB_DEV_INTERRUPT_DRIVEN
            asm("wfi")    ;
#else
            usb_dev.poll();
#endif
            sys_tick_timer.update64();
        }

        gpioc->bsrr = Gpio::Bsrr::BS13;   // set high to turn off LED
        while (sys_tick_timer.elapsed64() < MIDI_NOTE_OFF_TIME)
            asm("nop");

        if (++midi_note_ndx >= NUM_MIDI_NOTES)
              midi_note_ndx = 0;
    }

}  // main()
