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


#include <stm32f103xb.h>



void usb_mcu_init()
{
#ifdef USB_DEV_FLASH_WAIT_STATES
#if USB_DEV_FLASH_WAIT_STATES == 1
    // enable flash prefetch buffer, one wait state
    FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_0;
#elif USB_DEV_FLASH_WAIT_STATES == 2
    // enable flash prefetch buffer, two wait states
    FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;
#endif
#endif

    RCC->CR |= RCC_CR_HSEON;
    while(!(RCC->CR & RCC_CR_HSERDY));

    RCC->CFGR |=   RCC_CFGR_HPRE_DIV1
                 | RCC_CFGR_PPRE2_DIV1
                 | RCC_CFGR_PPRE1_DIV1 ;

    RCC->CFGR =    (RCC->CFGR & ~(RCC_CFGR_PLLSRC_Msk | RCC_CFGR_PLLMULL_Msk))
                 |  RCC_CFGR_PLLSRC     // PLL input from HSE (8 MHz)
                 |  RCC_CFGR_PLLMULL_0  // Multiply by 9 (8*9=72 MHz)
                 |  RCC_CFGR_PLLMULL_1
                 |  RCC_CFGR_PLLMULL_2;

    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    RCC->CFGR =   (RCC->CFGR & ~(RCC_CFGR_SW_HSE | RCC_CFGR_SW_PLL))
                | RCC_CFGR_SW_PLL;

    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_1)
        asm("nop");

    RCC->CFGR &= ~RCC_CFGR_USBPRE;      // clear for 1.5x USB prescaler vs 1.0x

    RCC->APB1ENR |= RCC_APB1ENR_USBEN;  // enable USB peripheral

    // enable GPIOs and alternate functions
    RCC->APB2ENR |=   RCC_APB2ENR_IOPAEN
                    | RCC_APB2ENR_IOPCEN
                    | RCC_APB2ENR_AFIOEN;

}  // usb_mcu_init()
