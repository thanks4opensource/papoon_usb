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


#include <usb_dev.hxx>

#if USB_DEV_MAJOR_VERSION == 1
#if USB_DEV_MINOR_VERSION  < 0
#warning USB_DEV_MINOR_VERSION < 0 with required USB_DEV_MAJOR_VERSION == 1
#endif
#else
#error USB_DEV_MAJOR_VERSION != 1
#endif


extern "C" {
void            usb_dev_serial_number_init() { usb_dev.serial_number_init(); }

int             usb_dev_init() { return usb_dev.init(); }

unsigned        usb_dev_device_state() { return (int)usb_dev.device_state(); }

void            usb_dev_interrupt_handler() { usb_dev.interrupt_handler(); }


#ifndef USB_DEV_INTERRUPT_DRIVEN
uint32_t        usb_dev_poll() { return usb_dev.poll(); }

uint32_t usb_dev_poll_recv_ready(
const uint8_t   endpoint)
{
    return usb_dev.poll_recv_ready(endpoint);
}

uint32_t usb_dev_poll_send_ready(
const uint8_t   endpoint)
{
    return usb_dev.poll_send_ready(endpoint);
}
#endif   /* ifndef USB_DEV_INTERRUPT_DRIVEN */


#ifdef USB_DEV_ENDPOINT_CALLBACKS
void usb_dev_register_recv_callback(
void     (*callback)(const uint8_t,
                     void*         ),
uint8_t   endpoint                  ,        // not checked
void     *user_data                 )
{
    usb_dev.register_recv_callback(callback, endpoint, user_data);
}

void usb_dev_register_send_callback(
void     (*callback)(const uint8_t,
                     void*         ),
uint8_t   endpoint                  ,        // not checked
void     *user_data                 )
{
    usb_dev.register_send_callback(callback, endpoint, user_data);
}
#endif   /* ifdef USB_DEV_ENDPOINT_CALLBACKS */


uint16_t usb_dev_endpoint_recv_bufsize(
const uint8_t endpoint)
{
    return usb_dev.endpoint_recv_bufsize(endpoint);
}

uint16_t usb_dev_endpoint_send_bufsize(
const uint8_t endpoint)
{
    return usb_dev.endpoint_send_bufsize(endpoint);
}


uint16_t usb_dev_recv_readys() { return usb_dev.recv_readys(); }
uint16_t usb_dev_send_readys() { return usb_dev.send_readys(); }


/* no checking of params -- caller must guarantee valid
                            endpoint_number and buffer
*/
uint16_t usb_dev_recv(
const uint8_t           endpoint_number,
      uint8_t* const    buffer         )
{
    return usb_dev.recv(endpoint_number, buffer);
}

/* no checking of params -- caller must guarantee valid
                            endpoint_number, data, and length
*/
int usb_dev_send(
const uint8_t           endpoint_number,
const uint8_t* const    data  ,
const uint16_t          length)
{
    return usb_dev.send(endpoint_number, data, length);
}

}  /* extern "C" */
