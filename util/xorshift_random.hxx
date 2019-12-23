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


#ifndef XORSHIFT_RANDOM_HXX
#define XORSHIFT_RANDOM_HXX

#include <stdint.h>

namespace bitops {

#ifndef BITOPS_XORSHIFT_RANDOM_BYTES
// client holds state
inline uint32_t xor_shift(
uint32_t &state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state <<  5;
    return  state;
}
#endif



class XorShift {
  public:
    constexpr
    XorShift(
    uint32_t    seed = 1)   // must be non-zero
    :   _state(seed)
#ifdef BITOPS_XORSHIFT_RANDOM_BYTES
                    ,
        _byte (0   )
#endif
    {}

    void seed(const uint32_t seed) {
        _state = seed;
#ifdef BITOPS_XORSHIFT_RANDOM_BYTES
        _byte  = 0;
#endif
    }

    uint32_t state() const { return _state; }

    uint32_t word() {
        _state ^= _state << 13;
        _state ^= _state >> 17;
        _state ^= _state <<  5;
        return _state;
    }

#ifdef BITOPS_XORSHIFT_RANDOM_BYTES
    uint8_t byte() {
        if ((_byte & 0x3) == 0) {
            word();
            _byte = 0;
        }
        return (_state >> (_byte++ << 3)) & 0xff;
    }
#else
    uint32_t operator()() { return word(); }
#endif


  protected:
    uint32_t    _state;
#ifdef BITOPS_XORSHIFT_RANDOM_BYTES
    unsigned    _byte;
#endif

};

} // namespace bitops

#endif   // ifndef XORSHIFT_RANDOM_HXX
