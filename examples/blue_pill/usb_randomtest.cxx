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


#include <core_cm3.hxx>

#include <stm32f103xb.hxx>

#include <sys_tick_timer.hxx>

#include <usb_mcu_init.hxx>

#include <usb_dev.hxx>

#include <random_test.hxx>



#if !defined(UP_MAX_PACKET_SIZE) || !defined(DOWN_MAX_PACKET_SIZE)
#error must define UP_MAX_PACKET_SIZE and DOWN_MAX_PACKET_SIZE
#endif

#if    UP_MAX_PACKET_SIZE !=    2 \
    && UP_MAX_PACKET_SIZE !=    4 \
    && UP_MAX_PACKET_SIZE !=    8 \
    && UP_MAX_PACKET_SIZE !=   16 \
    && UP_MAX_PACKET_SIZE !=   32 \
    && UP_MAX_PACKET_SIZE !=   64 \
    && UP_MAX_PACKET_SIZE !=  128 \
    && UP_MAX_PACKET_SIZE !=  256 \
    && UP_MAX_PACKET_SIZE !=  512 \
    && UP_MAX_PACKET_SIZE != 1024
#error UP_MAX_PACKET_SIZE must be even power of 2
#endif

#if DOWN_MAX_PACKET_SIZE % 4 != 0
#error DOWN_MAX_PACKET_SIZE must be evenly divisible by 4
#endif

#if !defined(USB_RANDOMTEST_UP_SEED) || !defined(USB_RANDOMTEST_DOWN_SEED)
#error must define USB_RANDOMTEST_UP_SEED and USB_RANDOMTEST_DOWN_SEED
#endif

#ifndef USB_RANDOMTEST_SYNC_LENGTH
#error must define USB_RANDOMTEST_SYNC_LENGTH
#endif


#ifndef USB_RANDOMTEST_LENGTH_SEED
#error must define USB_RANDOMTEST_LENGTH_SEED
#endif

#ifndef HISTOGRAM_LENGTH
#error must define HISTOGRAM_LENGTH
#endif


using namespace stm32f103xb;
using namespace stm32f10_12357_xx;


extern UsbDev   usb_dev;


namespace {
static const uint32_t   CPU_HZ           = 72000000,
                        LED_DELAY_TICK   = CPU_HZ / 50; // 0.020 seconds


unittest::RandomTest<HISTOGRAM_LENGTH>  random_test(USB_RANDOMTEST_DOWN_SEED  ,
                                                    USB_RANDOMTEST_UP_SEED    ,
                                                    USB_RANDOMTEST_LENGTH_SEED,
                                                    UP_MAX_PACKET_SIZE        ,
                                                    UP_MAX_PACKET_SIZE        );

arm::SysTickTimer   sys_tick_timer;

uint8_t     down_buf[DOWN_MAX_PACKET_SIZE],
              up_buf[  UP_MAX_PACKET_SIZE];

}  // namespace



#ifdef USB_DEV_INTERRUPT_DRIVEN
extern "C" void USB_LP_CAN1_RX0_IRQHandler()
{
    usb_dev.interrupt_handler();
}
#endif



using namespace stm32f103xb;
using namespace stm32f10_12357_xx;


namespace usb_randomtest {

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
#ifdef USB_DEV_INTERRUPT_DRIVEN
        asm("wfi");
#else
        usb_dev.poll();
#endif

    gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED
}



void sync_callback(
const uint8_t    endpoint ,
void            *user_data)
{
    gpioc->bsrr = Gpio::Bsrr::BR13;  // set low to turn on user LED

#ifdef USB_DEV_NO_BUFFER_RECV_SEND
    uint16_t    recv_len = usb_dev.recv_lnth(endpoint);

    for (uint16_t ndx = 0 ; ndx < (recv_len + 1) >> 1 ; ++ndx)
        reinterpret_cast<uint16_t*>(down_buf)[ndx] = usb_dev.read(endpoint, ndx);

    usb_dev.recv_done(endpoint);
#else
    uint16_t    recv_len = usb_dev.recv(endpoint, down_buf);
#endif
    random_test.recv_sync(down_buf, recv_len, USB_RANDOMTEST_SYNC_LENGTH);

    gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED
}



void recv_callback(
const uint8_t    endpoint ,
void            *user_data)
{
    gpioc->bsrr = Gpio::Bsrr::BR13;  // low to turn on user LED

#ifdef USB_DEV_NO_BUFFER_RECV_SEND
    uint16_t    recv_len = usb_dev.recv_lnth(endpoint);

    for (uint16_t ndx = 0 ; ndx < (recv_len + 1) >> 1 ; ++ndx)
        reinterpret_cast<uint16_t*>(down_buf)[ndx] = usb_dev.read(endpoint, ndx);

    usb_dev.recv_done(endpoint);
#else
    uint16_t    recv_len = usb_dev.recv(endpoint, down_buf);
#endif

    if (!random_test.recv(down_buf, recv_len))
        while (true)
            asm("nop");

    gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED
}



void send_callback(
const uint8_t    endpoint ,
void            *user_data)
{
    gpioc->bsrr = Gpio::Bsrr::BR13;  // low to turn on user LED

    uint16_t    send_len = random_test.send(up_buf);

    if (send_len) {
#ifdef USB_DEV_NO_BUFFER_RECV_SEND
        for (uint16_t ndx = 0 ; ndx < (send_len + 1) >> 1 ; ++ndx)
            usb_dev.writ(endpoint                                ,
                         reinterpret_cast<uint16_t*>(up_buf)[ndx],
                         ndx                                     );
        usb_dev.send(endpoint, send_len);
#else
        usb_dev.send(endpoint, up_buf, send_len);
#endif
    }

    gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED
}



void run(
const uint8_t    down_endpoint,
const uint8_t      up_endpoint)
{
    gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED

#ifdef USB_DEV_ENDPOINT_CALLBACKS
    usb_dev.register_recv_callback(sync_callback, down_endpoint, (void*)0);
#endif

    while (!random_test.synced()) {
#ifndef USB_DEV_INTERRUPT_DRIVEN
        usb_dev.poll();
#endif

#ifndef USB_DEV_ENDPOINT_CALLBACKS
        if (usb_dev.recv_ready(1 << down_endpoint))
            sync_callback(down_endpoint, (void*)0);
#endif
    }

#ifdef USB_DEV_ENDPOINT_CALLBACKS
    usb_dev.register_recv_callback(recv_callback, down_endpoint, (void*)0);
    usb_dev.register_send_callback(send_callback,   up_endpoint, (void*)0);
#endif

    while (true) {
        if (   !gpioc->odr.any(Gpio::Odr::ODR13)
            && sys_tick_timer.elapsed32() > LED_DELAY_TICK)
            gpioc->bsrr = Gpio::Bsrr::BS13;  // set high to turn off user LED

#ifndef USB_DEV_INTERRUPT_DRIVEN
        usb_dev.poll();
#endif

#ifndef USB_DEV_ENDPOINT_CALLBACKS
        if (usb_dev.recv_ready(1 << down_endpoint))
            recv_callback(down_endpoint, 0);

        if (usb_dev.send_ready(1 << up_endpoint))
            send_callback(up_endpoint, 0);
#endif
    }
}

}  // namespace usb_randomtest
