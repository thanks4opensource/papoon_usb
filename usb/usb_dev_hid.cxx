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


#include <usb_dev_hid.hxx>

namespace stm32f10_12357_xx {

const uint8_t UsbDevHid::_QUALIFIER_DESC[] = {
    10,     // bLength: qualifier size
    static_cast<uint8_t>(UsbDev::Descriptor::DEVICE_QUALIFIER),
    0x00,   // undocumented configuration values
    0x02,   //      "             "         "
    0x00,   //      "             "         "
    0x00,   //      "             "         "
    0x00,   //      "             "         "
    0x40,   //      "             "         "
    0x01,   //      "             "         "
    0x00,   //      "             "         "
};




bool UsbDevHid::usb_dev_hid_device_class_setup()
{
    UsbDevHid   *usb_dev = static_cast<UsbDevHid*>(this);
#if 0  // not working
#ifdef __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    const uint8_t   *data;
#else
    const uint8_t   *data = 0;  // silence compiler warning
#endif
#if 0
#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif
#endif
    uint16_t         size;

    if (!  _setup_packet
         ->request_type
         . all(SetupPacket::RequestType:: RECIPIENT_INTERFACE))
        return false;

    if (_setup_packet->request_type.all(SetupPacket::RequestType::TYPE_CLASS)) {
        switch (_setup_packet->request) {
            case UsbDevHid::_REQ_SET_PROTOCOL:
                usb_dev->_protocol = _setup_packet->value.bytes.byte0;
                break;

            case UsbDevHid::_REQ_GET_PROTOCOL:
                data = &usb_dev->_protocol;
                size = 1         ;
                break;

            case UsbDevHid::_REQ_SET_IDLE:
                usb_dev->_idle_state = _setup_packet->value.bytes.byte0;
                break;

            case UsbDevHid::_REQ_GET_IDLE:
                data = &usb_dev->_idle_state;
                size = 1           ;
                break;

            default:
                return false;
        }

        goto SET_SEND_OR_RECV;
    }

    if (  !  _setup_packet
           ->request_type
           . all(SetupPacket::RequestType::TYPE_STANDARD)
        ||    (static_cast<SetupPacket::Request>(_setup_packet->request)
           != SetupPacket::Request::GET_DESCRIPTOR))
        return false;

    switch (_setup_packet->value.bytes.byte1) {
        case static_cast<uint8_t>(UsbDev::Descriptor::DEVICE_QUALIFIER):
            data =        UsbDevHid::_QUALIFIER_DESC ;
            size = sizeof(UsbDevHid::_QUALIFIER_DESC);
            break;

#if 0  // handled by derived class
        case UsbDevHid::HID_REPORT_DESC_TYPE:
            data =        UsbDevHid::_REPORT_DESC ;
            size = sizeof(UsbDevHid::_REPORT_DESC);
            break;

        case UsbDevHid::HID_DESCRIPTOR_TYPE:
            data =        UsbDevHid::_HID_DESC ;
            size = sizeof(UsbDevHid::_HID_DESC);
            break;
#endif

        default:
            return false;
    }

    SET_SEND_OR_RECV:
    if (_setup_packet->request_type.any(   SetupPacket
                                        ::RequestType
                                        ::DIR_DEV_TO_HOST))
        _send_info.set(data, size);
    else
        _recv_info.set(const_cast<uint8_t*>(data), size);

    return true;
}


void UsbDev::set_configuration() {}
void UsbDev::set_interface    () {}


}  // namespace stm32f10_12357_xx {
