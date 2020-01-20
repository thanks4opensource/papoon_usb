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


#ifndef USB_DEV_MAX_ENDPTS_HXX
#define USB_DEV_MAX_ENDPTS_HXX

#include <usb_dev.hxx>

#if USB_DEV_MAJOR_VERSION == 1
#if USB_DEV_MINOR_VERSION  < 0
#warning USB_DEV_MINOR_VERSION < 0 with required USB_DEV_MAJOR_VERSION == 1
#endif
#else
#error USB_DEV_MAJOR_VERSION != 1
#endif


namespace stm32f10_12357_xx {

class UsbDevMaxEndpts : public UsbDev
{
  public:
    static const uint8_t
        // have to be public for clients, static descriptors
        NUM_IN_OUT_ENDPOINTS    = 7,  // plus control endpt 0, hardware max is 8
        // random endpoint addresses and order in configuration descriptor
         IN_ENDPOINT_1           = UsbDev::ENDPOINT_DIR_IN |  7,
        OUT_ENDPOINT_1           =                            7,
         IN_ENDPOINT_2           = UsbDev::ENDPOINT_DIR_IN |  2,
        OUT_ENDPOINT_2           =                            2,
         IN_ENDPOINT_3           = UsbDev::ENDPOINT_DIR_IN | 13,
        OUT_ENDPOINT_3           =                           13,
         IN_ENDPOINT_4           = UsbDev::ENDPOINT_DIR_IN |  9,
        OUT_ENDPOINT_4           =                            9,
         IN_ENDPOINT_5           = UsbDev::ENDPOINT_DIR_IN |  1,
        OUT_ENDPOINT_5           =                            1,
         IN_ENDPOINT_6           = UsbDev::ENDPOINT_DIR_IN | 15,
        OUT_ENDPOINT_6           =                           15,
         IN_ENDPOINT_7           = UsbDev::ENDPOINT_DIR_IN |  5,
        OUT_ENDPOINT_7           =                            5,
        // have to be public for extern static definition
         IN_ENDPOINTS_MAX_PACKET = 16,  // PMA: (512-2*64-8*16)/14=18.28571...
        OUT_ENDPOINTS_MAX_PACKET = IN_ENDPOINTS_MAX_PACKET,
         IN_ENDPOINTS_INTERVAL  =  1,  // frames @ 1 ms each
        OUT_ENDPOINTS_INTERVAL  =  1;  // frames @ 1 ms each

    // for client access to above endpoint order->addresses
    // has OUT endpoints, i.e. without UsbDev::ENDPOINT_DIR_IN, as
    //   per UsbDev client APIs
    static const uint8_t    ENDPOINT_ADDRESSES[NUM_IN_OUT_ENDPOINTS];

    constexpr UsbDevMaxEndpts()
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

};  // class UsbDevMaxEndpts

}  // namespace stm32f10_12357_xx

#endif  // ifndef USB_DEV_MAX_ENDPTS_HXX
