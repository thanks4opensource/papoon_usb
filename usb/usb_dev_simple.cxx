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


#include <usb_dev_simple.hxx>

namespace stm32f10_12357_xx {

const uint8_t UsbDev::_DEVICE_DESC[] = {
    0x12,   // bLength
    static_cast<uint8_t>(UsbDev::DescriptorType::DEVICE),
    0x00,
    0x02,   // bcdUSB = 2.00
    0x02,   // bDeviceClass: CDC
    0x00,   // bDeviceSubClass
    0x00,   // bDeviceProtocol
    0x40,   // bMaxPacketSize0
    0x83,   // idVendor = 0x0483
    0x04,   //    "     = MSB of uint16_t
    0xe3,   // idProduct = 0x62e3 (randomly chosen, not in Linux usb_ids.txt
    0x62,   //     "     = MSB of uint16_t
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
    0,      // wTotalLength: including sub-descriptors, will be set a runtime
    0x00,   //      "      : MSB of uint16_t
    0x01,   // bNumInterfaces: 1 interface
    0x01,   // bConfigurationValue: Configuration value
    0x00,   // iConfiguration: string descriptor index: none
    0xC0,   // bmAttributes: self powered
    0x32,   // MaxPower 100 mA (value==mA*0.5)

    // Interface Descriptor
    0x09,   // bLength: Interface Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::INTERFACE),
    0x00,   // bInterfaceNumber: Number of Interface
    0x00,   // bAlternateSetting: Alternate setting
    0x02,   // bNumEndpoints: 2
    0xff,   // bInterfaceClass: vendor specific
    0x00,   // bInterfaceSubClass: not used
    0xff,   // bInterfaceProtocol: vendor specific
    0x00,   // iInterface: string descriptor index: none

    // IN endpoint
    0x07,   // bLength: Endpoint Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::ENDPOINT), // bDescriptorType
    UsbDevSimple::IN_ENDPOINT | UsbDev::ENDPOINT_DIR_IN,    // bEndpointAddress
    static_cast<uint8_t>(UsbDev::EndpointType::INTERRUPT),  // bmAttributes
    UsbDevSimple::IN_ENDPOINT_MAX_PACKET,   // wMaxPacketSize: 64 bytes
    0x00,                                   //       "       : MSB of uint16_t
    UsbDevSimple::IN_ENDPOINT_INTERVAL,     // bInterval: set in .hxx file

    // OUT endpoint
    0x07,   // bLength: Endpoint Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::ENDPOINT), // bDescriptorType
    UsbDevSimple::OUT_ENDPOINT,                             // bEndpointAddress
    static_cast<uint8_t>(UsbDev::EndpointType::INTERRUPT),  // bmAttributes
    UsbDevSimple::OUT_ENDPOINT_MAX_PACKET,  // wMaxPacketSize: 64 bytes
    0x00,                                   //       "       : MSB of uint16_t
    UsbDevSimple::OUT_ENDPOINT_INTERVAL,    // bInterval: set in .hxx file
};

const uint8_t   UsbDevSimple::_device_string_desc[] = {
                34,
                static_cast<uint8_t>(UsbDev::DescriptorType::STRING),
                'S', 0, 'T', 0, 'M', 0, '3', 0,
                '2', 0, ' ', 0, 'S', 0, 'i', 0,
                'm', 0, 'p', 0, 'l', 0, 'e', 0,
                ' ', 0, 'U', 0, 'S', 0, 'B', 0};     // "STM32 Simple USB"

const uint8_t   *UsbDev::_STRING_DESCS[] = {
    UsbDev      ::  language_id_string_desc(),
    UsbDev      ::       vendor_string_desc(),
    UsbDevSimple::       device_string_desc(),
    UsbDev      ::serial_number_string_desc(),
};



bool UsbDevSimple::init()
{
    _CONFIG_DESC[UsbDev::CONFIG_DESC_SIZE_NDX]  = sizeof(_CONFIG_DESC);

    return UsbDev::init();
}



bool UsbDev::device_class_setup()
{
    return false;  // no class-specific setup
}


void UsbDev::set_configuration() {}
void UsbDev::set_interface    () {}


}  // namespace stm32f10_12357_xx {
