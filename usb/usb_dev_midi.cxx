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


#include <usb_dev_midi.hxx>

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
    0x10,   // idProduct = 0x7510 "Joystick in FS Mode"
    0x57,   //     "     = MSB of uint16_t
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
    0,      // wTotalLength: including sub-descriptors; will be set a runtime
    0x00,   //      "      : MSB of uint16_t
    0x02,   // bNumInterfaces: 1 interface
    0x01,   // bConfigurationValue: Configuration value
    0x00,   // iConfiguration: Index of string descriptor for configuration
    0x80,   // bmAttributes: self powered
    0x32,   // MaxPower 100 mA (2*value)


    // Audio Control Interface Descriptors
    //

    // Standard Interface Descriptor: Audio Control
    0x09,   // bLength: Interface Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::INTERFACE), // bDescriptorType
    0x00,   // bInterfaceNumber: index of this interface = 0
    0x00,   // bAlternateSetting: Alternate setting
    0x00,   // bNumEndpoints: number of endpoints: 0
    0x01,   // bInterfaceClass: Audio
    0x01,   // bInterfaceSubClass: Audio Control
    0x00,   // bInterfaceProtocol: unused
    0x00,   // iInterface: index of string descriptor, unused

    // Class-specific Interface Descriptor: Audio Control
    0x09,   // bLength: length of descriptor: 9
    static_cast<uint8_t>(UsbDevMidi::CLASS_SPECIFIC_INTERFACE_DESCRIPTOR__TYPE),
    0x01,   // bDescriptorSubtype: HEADER: 1
    0x01,   // bcdADC class specification revision: 1.0
    0x00,   //   "      "         "          "    : MSBs of uint16_t
    0x09,   // wTotalLength: 9
    0x00,   //      "      : MSBs of uint16_t
    0x01,   // binCollection: Number of streaming interfaces: 1
    0x01,   // baInterfaceNum: MIDIStreaming interface 1 belongs to interface


    // MIDIStreaming Interface Descriptors
    //

    // Standard Interface Descriptor: MIDIStreaming
    0x09,   // bLength: length of descriptor: 9
    static_cast<uint8_t>(UsbDev::DescriptorType::INTERFACE), // bDescriptorType
    0x01,   // bInterfaceNumber: index of this interface: 1
    0x00,   // bAlternateSetting: Alternate setting
    0x02,   // bNumEndpoints: number of endpoints for this interface: 2
    0x01,   // bInterfaceClass: Audio
    0x03,   // bInterfaceSubClass: MIDIStreaming
    0x00,   // bInterfaceProtocol: unused
    0x00,   // iInterface: index of string descriptor, unused

    // Class-specific Interface Descriptor: MIDIStreaming
    0x07,   // bLength: length of descriptor: 7
    static_cast<uint8_t>(UsbDevMidi::CLASS_SPECIFIC_INTERFACE_DESCRIPTOR__TYPE),
    0x01,   // bDescriptorSubtype: MIDIStreaming Header subtype: 1
    0x01,   // bcdADC class specification revision: 1.0
    0x00,   //   "      "         "          "    : MSB of uint16_t
    0x41,   // wTotalLength: total size of class-specific descriptors: 65
    0x00,   //      "      : MSBs of uint16_t

    // MIDI Adapter MIDI IN Jack Descriptor (Embedded)
    0x06,   // bLength: length of descriptor: 9
    static_cast<uint8_t>(UsbDevMidi::CLASS_SPECIFIC_INTERFACE_DESCRIPTOR__TYPE),
    0x02,   // bDescriptorSubtype: MIDI_IN_JACK subtype: 2
    0x01,   // bJackType: EMBEDDED: 1
    0x01,   // bJackID: jack ID: 1
    0x00,   // iJack: unused

    // MIDI Adapter MIDI IN Jack Descriptor (External)
    0x06,   // bLength: length of descriptor: 9
    static_cast<uint8_t>(UsbDevMidi::CLASS_SPECIFIC_INTERFACE_DESCRIPTOR__TYPE),
    0x02,   // bDescriptorSubtype: MIDI_IN_JACK subtype: 2
    0x02,   // bJackType: EXTERNAL: 2
    0x02,   // bJackID: jack ID: 2
    0x00,   // iJack: unused

    // MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
    0x09,   // bLength: length of descriptor: 9
    static_cast<uint8_t>(UsbDevMidi::CLASS_SPECIFIC_INTERFACE_DESCRIPTOR__TYPE),
    0x03,   // bDescriptorSubtype: MIDI_OUT_JACK subtype: 3
    0x01,   // bJackType: EMBEDDED: 1
    0x03,   // bJackID: jack ID: 3
    0x01,   // bNrInputPins: number of input pins: 1
    0x02,   // BaSourceID: ID of entity to which pin is connected: 2
    0x01,   // BaSourcePin: Output pin number of entity pin connected to: 1
    0x00,   // iJack: unused

    // MIDI Adapter MIDI OUT Jack Descriptor (External)
    0x09,   // bLength: length of descriptor: 9
    static_cast<uint8_t>(UsbDevMidi::CLASS_SPECIFIC_INTERFACE_DESCRIPTOR__TYPE),
    0x03,   // bDescriptorSubtype: MIDI_OUT_JACK subtype: 3
    0x02,   // bJackType: EXTERNAL: 2
    0x04,   // bJackID: jack ID: 4
    0x01,   // bNrInputPins: number of input pins: 1
    0x01,   // BaSourceID: ID of entity to which pin is connected: 1
    0x01,   // BaSourcePin: Output pin number of entity pin connected to: 1
    0x00,   // iJack: unused


    // Endpoint Descriptors
    //

    // Standard Bulk OUT Endpoint Descriptor
    0x09,   // bLength: length of descriptor: 9
    static_cast<uint8_t>(UsbDev::DescriptorType::ENDPOINT),// bDescriptorType
    static_cast<uint8_t>(UsbDevMidi::BULK_OUT_ENDPOINT),   // bEndpointAddress:1
    static_cast<uint8_t>(UsbDev::EndpointType::BULK),      // bmAttributes
    0x40,   // wMaxPacketSize: 64
    0x00,   //       "       : MSB of uint16_t
    0x00,   // bInterval: ignored for BULK endpoints
    0x00,   // bRefresh: unused
    0x00,   // bSyncAddress: unused

    // Class-specific Bulk OUT Endpoint Descriptor
    0x05,   // bLength: length of descriptor: 5
    static_cast<uint8_t>(UsbDevMidi::CLASS_SPECIFIC_ENDPOINT_DESCRIPTOR__TYPE),
    0x01,   // bDescriptorSubtype: MS_GENERAL subtype: 1
    0x01,   // bNumEmbMIDIJack: number of embedded MIDI IN jacks
    0x01,   // BaAssocJackID(1): ID of associated MIDI OUT jack: 1

    // Standard Bulk IN Endpoint Descriptor
    0x09,   // bLength: length of descriptor: 9
    static_cast<uint8_t>(UsbDev::DescriptorType::ENDPOINT),// bDescriptorType
    UsbDevMidi::BULK_IN_ENDPOINT | UsbDev::ENDPOINT_DIR_IN,// bEndpointAddress:1
    static_cast<uint8_t>(UsbDev::EndpointType::BULK),      // bmAttributes
    0x40,   // wMaxPacketSize: 64
    0x00,   //       "       : MSB of uint16_t
    0x00,   // bInterval: ignored for BULK endpoints
    0x00,   // bRefresh: unused
    0x00,   // bSyncAddress: unused

    // Class-specific Bulk IN Endpoint Descriptor
    0x05,   // bLength: length of descriptor: 5
    static_cast<uint8_t>(UsbDevMidi::CLASS_SPECIFIC_ENDPOINT_DESCRIPTOR__TYPE),
    0x01,   // bDescriptorSubtype: MS_GENERAL subtype: 1
    0x01,   // bNumEmbMIDIJack: number of embedded MIDI OUT jacks
    0x03,   // BaAssocJackID(1): ID of associated MIDI OUT jack: 3
};

const uint8_t UsbDevMidi::_QUALIFIER_DESC[] = {
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

const uint8_t   UsbDevMidi::_device_string_desc[] = {
                22,
                static_cast<uint8_t>(UsbDev::DescriptorType::STRING),
                'S', 0, 'T', 0, 'M', 0, '3', 0,
                '2', 0, ' ', 0, 'M', 0, 'I', 0,
                'D', 0, 'I', 0                };  // STM32 MIDI


const uint8_t   *UsbDev::_STRING_DESCS[] = {
    UsbDev    ::  language_id_string_desc(),
    UsbDev    ::       vendor_string_desc(),
    UsbDevMidi::       device_string_desc(),
    UsbDev    ::serial_number_string_desc(),
};




bool UsbDevMidi::init()
{
    _CONFIG_DESC[UsbDev::CONFIG_DESC_SIZE_NDX]  = sizeof(_CONFIG_DESC);

    return UsbDev::init();
}



bool UsbDev::device_class_setup()
{
#if 0
    UsbDevMidi  *self = static_cast<UsbDevHidMouse*>(this);
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
            case UsbDevHidMouse::_REQ_SET_PROTOCOL:
                self->_protocol = _setup_packet->value.bytes.byte0;  // byte1 ??
                break;

            case UsbDevHidMouse::_REQ_GET_PROTOCOL:
                data = &self->_protocol;
                size = 1         ;
                break;

            case UsbDevHidMouse::_REQ_SET_IDLE:
                self->_idle_state = _setup_packet->value.bytes.byte0;  // byte1??
                break;

            case UsbDevHidMouse::_REQ_GET_IDLE:
                data = &self->_idle_state;
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
            data =        UsbDevHidMouse::_QUALIFIER_DESC ;
            size = sizeof(UsbDevHidMouse::_QUALIFIER_DESC);
            break;

        case UsbDevHidMouse::HID_REPORT_DESC_TYPE:
            data =        UsbDevHidMouse::_REPORT_DESC ;
            size = sizeof(UsbDevHidMouse::_REPORT_DESC);
            break;

        case UsbDevHidMouse::HID_DESCRIPTOR_TYPE:
            data =        UsbDevHidMouse::_HID_DESC ;
            size = sizeof(UsbDevHidMouse::_HID_DESC);
            break;

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
#else
    return false;
#endif
}


void UsbDev::set_configuration() {}
void UsbDev::set_interface    () {}


}  // namespace stm32f10_12357_xx {
