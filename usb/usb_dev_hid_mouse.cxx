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


#include <usb_dev_hid_mouse.hxx>

namespace stm32f10_12357_xx {

const uint8_t UsbDev::_DEVICE_DESC[] = {
    0x12,   // bLength
    static_cast<uint8_t>(UsbDev::DescriptorType::DEVICE),   // bDescriptorType
    0x00,
    0x02,   // bcdUSB = 2.00
    0x02,   // bDeviceClass: CDC
    0x00,   // bDeviceSubClass
    0x00,   // bDeviceProtocol
    0x40,   // bMaxPacketSize0
    0x83,   // idVendor = 0x0483
    0x04,   //    "     = MSB of uint16_t
    0x10,   // idProduct = 0x5710 "Joystick in FS Mode"
    0x57,   //      "     = MSB of uint16_t
    0x00,   // bcdDevice = 2.00
    0x02,   //     "     = MSB of uint16_t
    1,      // Index of string descriptor describing manufacturer
    2,      // Index of string descriptor describing product
    3,      // Index of string descriptor describing device serial number
    0x01    // bNumConfigurations
};

uint8_t UsbDev::_CONFIG_DESC[] = {  // not const because set total size entry
    // Configuration Descriptor
    0x09,   // bLength: Configuration Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::CONFIGURATION),
    0,      // wTotalLength: including sub-descriptors; will be set at runtime
    0x00,   //      "      : MSB of uint16_t
    0x01,   // bNumInterfaces: 1 interface
    0x01,   // bConfigurationValue: Configuration value
    0x00,   // iConfiguration: Index of string descriptor for configuration
    0x80,   // bmAttributes: self powered
    0x32,   // MaxPower 0 mA

    // Interface Descriptor
    0x09,   // bLength: Interface Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::INTERFACE), // bDescriptorType
    0x00,   // bInterfaceNumber: Number of Interface
    0x00,   // bAlternateSetting: Alternate setting
    0x01,   // bNumEndpoints: One endpoints used
    0x03,   // bInterfaceClass: HID (Human Interface Device)
    0x01,   // bInterfaceSubClass: Boot Interface SubClass
    0x02,   // bInterfaceProtocol: Mouse Protocol
    0x00,   // iInterface: index of string descriptor?

    // HID Mouse Descriptor
    0x09,   // bLength: HID Descriptor size
    static_cast<uint8_t>(UsbDevHidMouse::HID_DESCRIPTOR_TYPE),// bDescriptorType
    0x11,   // bcdHID: HID Class Spec release number 0x0111==1.11
    0x01,   //   "   : MSB of uint16_t
    0x00,   // bCountryCode: Hardware target country (0==none)
    0x01,   // bNumDescriptors: Number of HID class descriptors to follow
    UsbDevHidMouse::HID_REPORT_DESC_TYPE,   // bDescriptorType
    UsbDevHidMouse::MOUSE_REPORT_DESC_SIZE, // wItemLength: of report descriptor
    0x00,   // high byte of _MOUSE_REPORT_DESC_SIZE

    // Endpoint Descriptor
    0x07,          // bLength: Endpoint Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::ENDPOINT), // bDescriptorType:
    UsbDev::ENDPOINT_DIR_IN | UsbDevHidMouse::MOUSE_ENDPOINT_IN,// endpt 1, IN
    static_cast<uint8_t>(UsbDev::EndpointType::INTERRUPT),  // bmAttributes
    UsbDevHidMouse::MOUSE_REPORT_SIZE,  // wMaxPacketSize: 4 Byte max
    0x00,                               //       "       : MSB of uint16_t
    UsbDevHidMouse::IN_FS_POLLING_INTERVAL, // bInterval: 10 ms
};

const uint8_t UsbDevHid::_HID_DESC[] = {
    0x09,   // bLength: HID Descriptor size
    HID_DESCRIPTOR_TYPE, // bDescriptorType: HID
    0x11,   // bcdHID: HID Class Spec release number (0x0111==1.11)
    0x01,   //   "   : MSB of uint16_t
    0x00,   // bCountryCode: Hardware target country (0==none)
    0x01,   // bNumDescriptors: Number of HID class descriptors to follow
    UsbDevHidMouse::HID_REPORT_DESC_TYPE,       // bDescriptorType
    UsbDevHidMouse::MOUSE_REPORT_DESC_SIZE,// wItemLength: Total length of Report descriptor
    0x00,       // high byte of _MOUSE_REPORT_DESC_SIZE
};

const uint8_t UsbDevHid::_REPORT_DESC[] = {
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x02, // Usage (Mouse)
    0xa1, 0x01, // Collection (Application)
    0x09, 0x01, // Usage (Pointer)

    0xa1, 0x00, // Collection (Physical)
    0x05, 0x09, // Usage Page (Buttons)
    0x19, 0x01, // Usage Minimum (01)
    0x29, 0x01, // Usage Maximum (01)

    0x15, 0x00, // Logical Minimum (0)
    0x25, 0x01, // Logical Maximum (1)
    0x95, 0x03, // Report Count (3)
    0x75, 0x01, // Report Size (1)

    0x81, 0x02, // Input (Data, Variable, Absolute)
    0x95, 0x01, // Report Count (1)
    0x75, 0x05, // Report Size (5)
    0x81, 0x01, // Input (Constant) for padding

    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x30, // Usage (X)
    0x09, 0x31, // Usage (Y)
    0x09, 0x38, // Logical Minimum (-127)

    0x15, 0x81, // Logical Maximum (127 (??))
    0x25, 0x7f, // Report Size (8)(??)
    0x75, 0x08, // Report Count (2)(??)
    0x95, 0x03, // Input (Data, Variable, Relative) (??)

    // undocumented configuration values
    0x81,     0x06,
    0xc0,     0x09,
    0x3c,     0x05,
    0xff,     0x09,

    0x01,     0x15,
    0x00,     0x25,
    0x01,     0x75,
    0x01,     0x95,

    0x02,     0xb1,
    0x22,     0x75,
    0x06,     0x95,
    0x01,     0xb1,

    0x01,     0xc0
};

const uint8_t   UsbDevHid::_device_string_desc[] = {
                32,
                static_cast<uint8_t>(UsbDev::DescriptorType::STRING),
                'S', 0, 'T', 0, 'M', 0, '3', 0,
                '2', 0, ' ', 0, 'H', 0, 'I', 0,
                'D', 0, ' ', 0, 'm', 0, 'o', 0,
                'u', 0, 's', 0, 'e', 0,       };  // STM32 HID mouse


const uint8_t   *UsbDev::_STRING_DESCS[] = {
    UsbDev        ::  language_id_string_desc(),
    UsbDev        ::       vendor_string_desc(),
    UsbDevHidMouse::       device_string_desc(),
    UsbDev        ::serial_number_string_desc(),
};




bool UsbDevHidMouse::init()
{
    _CONFIG_DESC[UsbDev::CONFIG_DESC_SIZE_NDX]  = sizeof(_CONFIG_DESC);

    return UsbDev::init();
}



bool UsbDev::device_class_setup()
{
    UsbDevHid   *usb_dev_hid = static_cast<UsbDevHid*>(this);

    if (usb_dev_hid->usb_dev_hid_device_class_setup())
        return true;

    if (  !  _setup_packet
           ->request_type
           . all(SetupPacket::RequestType::TYPE_STANDARD)
        ||    (static_cast<SetupPacket::Request>(_setup_packet->request)
           != SetupPacket::Request::GET_DESCRIPTOR))
        return false;

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

    switch (_setup_packet->value.bytes.byte1) {
        case UsbDevHid::HID_REPORT_DESC_TYPE:
            data =        UsbDevHid::_REPORT_DESC ;
            size = sizeof(UsbDevHid::_REPORT_DESC);
            break;

        case UsbDevHid::HID_DESCRIPTOR_TYPE:
            data =        UsbDevHid::_HID_DESC ;
            size = sizeof(UsbDevHid::_HID_DESC);
            break;

        default:
            return false;
    }

    if (_setup_packet->request_type.any(   SetupPacket
                                        ::RequestType
                                        ::DIR_DEV_TO_HOST))
        _send_info.set(data, size);
    else
        _recv_info.set(const_cast<uint8_t*>(data), size);

    return true;
}

}  // namespace stm32f10_12357_xx {
