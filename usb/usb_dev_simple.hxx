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


#ifndef USB_DEV_SIMPLE_HXX
#define USB_DEV_SIMPLE_HXX

#include <usb_dev.hxx>

#if USB_DEV_MAJOR_VERSION == 1
#if USB_DEV_MINOR_VERSION  < 0
#warning USB_DEV_MINOR_VERSION < 0 with required USB_DEV_MAJOR_VERSION == 1
#endif
#else
#error USB_DEV_MAJOR_VERSION != 1
#endif


namespace stm32f10_12357_xx {

class UsbDevSimple : public UsbDev
{
  public:
    static const uint8_t    // have to be public for clients, static descriptors
                             IN_ENDPOINT            =  1,// 0x81 with DIR_IN bit
                            OUT_ENDPOINT            =  2,
                            // have to be public for extern static definition
                             IN_ENDPOINT_MAX_PACKET = 64,
                            OUT_ENDPOINT_MAX_PACKET = 64,
                             IN_ENDPOINT_INTERVAL   =  1,  // frames @ 1 ms each
                            OUT_ENDPOINT_INTERVAL   =  1;  // frames @ 1 ms each

    constexpr UsbDevSimple()
    :   UsbDev()
    {}

    bool init();


    // need public accessor for static initialization of _NEW_STRING_DESCS
    //
    static constexpr const uint8_t* device_string_desc()
    {
        return _device_string_desc;
    }




  protected:
    friend class UsbDev;

    struct LineCoding {
        uint32_t    baud       ;
        uint8_t     stop_bits  ,
                    parity_code,
                    bits       ;
    };

    static const uint8_t    _NUM_ENDPOINTS          = 3   ,
                            _SET_LINE_CODING        = 0x20,
                            _GET_LINE_CODING        = 0x21,
                            _SET_CONTROL_LINE_STATE = 0x22;

    static const uint8_t        _device_string_desc[];
    static       LineCoding     _line_coding         ;

};  // class UsbDevSimple

}  // namespace stm32f10_12357_xx

#endif  // ifndef USB_DEV_SIMPLE_HXX
