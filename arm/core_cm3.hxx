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


#ifndef CORE_CM3_HXX
#define CORE_CM3_HXX

#define CORE_CMX_HXX_INCLUDED

#include <stdint.h>

#include <regbits.hxx>

namespace arm {

struct SysTick {
    struct Ctrl {
        using            pos_t = regbits::Pos<uint32_t, Ctrl>;
        static constexpr pos_t
           COUNTFLAG_POS = pos_t(16),
           CLKSOURCE_POS = pos_t( 2),
             TICKINT_POS = pos_t( 1),
              ENABLE_POS = pos_t( 0);

        using            bits_t = regbits::Bits<uint32_t, Ctrl>;
        static constexpr bits_t
            COUNTFLAG        = bits_t(1,    COUNTFLAG_POS),
            CLKSOURCE        = bits_t(1,    CLKSOURCE_POS),
            TICKINT          = bits_t(1,      TICKINT_POS),
            ENABLE           = bits_t(1,       ENABLE_POS);
    };  // struct Ctrl
    using ctrl_t = regbits::Reg<uint32_t, Ctrl>;
          ctrl_t   ctrl;


    struct Load {
        static const uint32_t
                 RELOAD_MAX  = 0xFFFFFFUL;
        // just use operator=(uint32) to write and word() to read
        // bits 31:24 are guaranteed RAZ/WI
        // only implemented as Reg for RELOAD_MAX const
    };  // struct Load
    using load_t = regbits::Reg<uint32_t, Load>;
          load_t   load;


    // any MSBs > Load::RELOAD_MAX are RAZ
    // any write to register clears all bits
                 uint32_t   val;
    static const uint32_t   VAL_MAX = 0xffffff;


    struct Calib {
        using            pos_t = regbits::Pos<uint32_t, Calib>;
        static constexpr pos_t
               NOREF_POS = pos_t(31),
                SKEW_POS = pos_t(30),
               TENMS_POS = pos_t( 0);

        using            bits_t = regbits::Bits<uint32_t, Calib>;
        static constexpr bits_t
            NOREF            = bits_t(1,        NOREF_POS),
            SKEW             = bits_t(1,         SKEW_POS);

        static const uint32_t
                  TENMS_MASK = 0xFFFFFFUL;
    };  // struct Calib
    using calib_t = regbits::Reg<uint32_t, Calib>;
          calib_t   calib;

};  // struct SysTick
static_assert(sizeof(SysTick) == 16, "sizeof(SysTick) != 16");


struct Nvic {
    static const uint8_t    NUM_INTERRUPT_REGS =     8,
                            NUM_PRIORITY_REGS    = 240;

    volatile uint32_t   Iser  [NUM_INTERRUPT_REGS],
                        _rsrv0[                24],
                        Icer  [NUM_INTERRUPT_REGS],
                        _rsrv1[                24],
                        Ispr  [NUM_INTERRUPT_REGS],
                        _rsrv2[                24],
                        Icpr  [NUM_INTERRUPT_REGS],
                        _rsrv3[                24],
                        Iabr  [NUM_INTERRUPT_REGS],
                        _rsrv4[                56],
                        Ip    [NUM_PRIORITY_REGS ],
                        _rsrv5[               644],
                        Stir                      ;
};   // struct Nvic};    // struct Nvic



static const uint32_t   SCS_BASE       = 0xE000E000UL,
                        ITM_BASE       = 0xE0000000UL,
                        DWT_BASE       = 0xE0001000UL,
                        TPI_BASE       = 0xE0040000UL,
                        COREDEBUG_BASE = 0xE000EDF0UL;

static const uint32_t   SYSTICK_BASE = SCS_BASE + 0x0010UL,
                        NVIC_BASE    = SCS_BASE + 0x0100UL,
                        SCB_BASE     = SCS_BASE + 0x0D00UL;

static volatile SysTick* const
sys_tick = reinterpret_cast<volatile SysTick*>(SYSTICK_BASE);
static Nvic*    const   nvic    = reinterpret_cast<Nvic*   >(NVIC_BASE   );
#if 0
static Scb*     const   scb     = reinterpret_cast<Scb*    >(SCB_BASE    );
#endif

}  // namespace arm

#endif  // #ifndef CORE_CM3_HXX
