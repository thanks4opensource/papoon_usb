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


#ifndef USB_DEV_MIDI_HXX
#define USB_DEV_MIDI_HXX

#include <usb_dev.hxx>

#if USB_DEV_MAJOR_VERSION == 1
#if USB_DEV_MINOR_VERSION  < 0
#warning USB_DEV_MINOR_VERSION < 0 with required USB_DEV_MAJOR_VERSION == 1
#endif
#else
#error USB_DEV_MAJOR_VERSION != 1
#endif


namespace stm32f10_12357_xx {

class UsbDevMidi : public UsbDev
{
  public:
    static const uint8_t    // have to be public for clients, static descriptors
                            CLASS_SPECIFIC_INTERFACE_DESCRIPTOR__TYPE   = 0x24,
                            CLASS_SPECIFIC_ENDPOINT_DESCRIPTOR__TYPE    = 0x25,
                            BULK_OUT_ENDPOINT = 1,
                            BULK_IN_ENDPOINT  = 1;  // or'd with 0x80

    constexpr UsbDevMidi()
    :   UsbDev      ( ),
        _protocol   (0),
        _idle_state (0)
    {}

    bool init();


    // need public accessor for static initialization of _NEW_STRING_DESCS
    //
    static constexpr const uint8_t* device_string_desc()
    {
        return _device_string_desc;
    }



    struct EventPacket {
        enum CodeIndex {
              TWO_BYTE_SYSTEM_COMMON    = 0x2,
            THREE_BYTE_SYSTEM_COMMON    = 0x3,
            SYS_EX_START_OR_CONTINUE    = 0x4,
            SINGLE_BYTE_SYSTEM_COMMON   = 0x5,
            SYS_EX_ENDS_FOLLOWING_TWO   = 0x6,
            SYS_EX_ENDS_FOLLOWING_THREE = 0x7,
            NOTE_OFF                    = 0x8,
            NOTE_ON                     = 0x9,
            POLY_KEYPRESS               = 0xA,
        };

       constexpr EventPacket()
       :    _cable_code(0),
            _midi_0    (0),
            _midi_1    (0),
            _midi_2    (0)
        {}

       constexpr EventPacket(
       const uint8_t    cable_number,
       const CodeIndex  code_index  ,
       const uint8_t    midi_0      ,
       const uint8_t    midi_1      ,
       const uint8_t    midi_2      )
       :    _cable_code((cable_number << 4) | code_index),
            _midi_0    (midi_0                          ),
            _midi_1    (midi_1                          ),
            _midi_2    (midi_2                          )
        {}

        void populate(const uint8_t     cable_number,
                      const CodeIndex   code_index  ,
                      const uint8_t     midi_0      ,
                      const uint8_t     midi_1      ,
                      const uint8_t     midi_2      )
        volatile
        {
            _cable_code = (cable_number << 4) | code_index;
            _midi_0     = midi_0                          ;
            _midi_1     = midi_1                          ;
            _midi_2     = midi_2                          ;
        }

        operator const uint8_t*() volatile {
            return const_cast      <const uint8_t*>    (
                   reinterpret_cast<const uint8_t*>    (
                   const_cast      <const EventPacket*>(this)));
        }

        uint8_t     _cable_code,
                    _midi_0    ,
                    _midi_1    ,
                    _midi_2    ;
    };


  protected:
    friend class UsbDev;

    struct LineCoding {
        uint32_t    baud       ;
        uint8_t     stop_bits  ,
                    parity_code,
                    bits       ;
    };

    static const uint8_t
                            _REQ_SET_PROTOCOL       = 0x0b,
                            _REQ_GET_PROTOCOL       = 0x03,
                            _REQ_SET_IDLE           = 0x0a,
                            _REQ_GET_IDLE           = 0x02;

    static const uint8_t    _device_string_desc[],
                            _QUALIFIER_DESC    [],
                            _HID_DESC          [],
                            _REPORT_DESC       [];

    uint8_t     _protocol  ,
                _idle_state;


};  // class UsbDevMidi

}  // namespace stm32f10_12357_xx

#endif  // ifndef USB_DEV_MIDI_HXX
