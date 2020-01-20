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


#ifndef USB_DEV_HID_HXX
#define USB_DEV_HID_HXX

#include <usb_dev.hxx>

#if USB_DEV_MAJOR_VERSION == 1
#if USB_DEV_MINOR_VERSION  < 0
#warning USB_DEV_MINOR_VERSION < 0 with required USB_DEV_MAJOR_VERSION == 1
#endif
#else
#error USB_DEV_MAJOR_VERSION != 1
#endif


namespace stm32f10_12357_xx {

class UsbDevHid : public UsbDev
{
  public:
    static const uint8_t    // have to be public for clients, static descriptors
                            HID_DESCRIPTOR_TYPE      = 0x21,
                            HID_REPORT_DESC_TYPE     = 0x22,
                            MOUSE_ENDPOINT_IN        =    1,
                            MOUSE_REPORT_DESC_SIZE   =   74,
                            MOUSE_REPORT_SIZE        =    4,
                            IN_FS_POLLING_INTERVAL   =   10;

    constexpr UsbDevHid()
    :   UsbDev      ( ),
        _protocol   (0),
        _idle_state (0)
    {}


    // need public accessor for static initialization of _NEW_STRING_DESCS
    //
    static constexpr const uint8_t* device_string_desc()
    {
        return _device_string_desc;
    }




  protected:
    friend class UsbDev;

    static const uint8_t
                            _REQ_SET_PROTOCOL       = 0x0b,
                            _REQ_GET_PROTOCOL       = 0x03,
                            _REQ_SET_IDLE           = 0x0a,
                            _REQ_GET_IDLE           = 0x02;

    static const uint8_t    _device_string_desc[],
                            _QUALIFIER_DESC    [],
                            _HID_DESC          [],
                            _REPORT_DESC       [];


    // called by derived class UsbDev::device_class_setup()
    bool    usb_dev_hid_device_class_setup();


    uint8_t     _protocol  ,
                _idle_state;


};  // class UsbDevHid

}  // namespace stm32f10_12357_xx

#endif  // ifndef USB_DEV_HID_HXX
