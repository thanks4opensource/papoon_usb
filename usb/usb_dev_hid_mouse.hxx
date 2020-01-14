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


#ifndef USB_DEV_HID_MOUSE_HXX
#define USB_DEV_HID_MOUSE_HXX

#include <usb_dev_hid.hxx>

#if USB_DEV_MAJOR_VERSION == 1
#if USB_DEV_MINOR_VERSION  < 0
#warning USB_DEV_MINOR_VERSION < 0 with required USB_DEV_MAJOR_VERSION == 1
#endif
#else
#error USB_DEV_MAJOR_VERSION != 1
#endif


namespace stm32f10_12357_xx {

class UsbDevHidMouse : public UsbDevHid
{
  public:
    static const uint8_t    // have to be public for clients, static descriptors
                            MOUSE_ENDPOINT_IN        =    1,
                            MOUSE_REPORT_DESC_SIZE   =   74,
                            MOUSE_REPORT_SIZE        =    4;

    constexpr UsbDevHidMouse()
    :   UsbDevHid()
    {}

    bool init();


#if 0
  protected:
    friend class UsbDev;
#endif

};  // class UsbDevHidMouse

}  // namespace stm32f10_12357_xx

#endif  // ifndef USB_DEV_HID_MOUSE_HXX
