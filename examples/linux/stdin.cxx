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


#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <string>

#include <libusb.h>


namespace {
static const uint16_t   VENDOR         = 0x0483,      // usb_dev_xxx.cxx
                        PRODUCT        = 0x62e3,      //        "
                        MAX_IN_PACKET  = 64    ;      // usb_dev_simple.hxx

static const uint8_t    NUM_IN_OUT_ENDPOINTS =    7,  // usb_dev_max_endpts.hxx
                         IN_ENDPOINT         = 0x81,  // usb_dev_simple.hxx
                        OUT_ENDPOINT         = 0x02;  //         "

static const int        OUT_TIMEOUT = 1000,  // milliseconds? not documented
                         IN_TIMEOUT = 1000;  // in libusb.h

// see usb_dev_max_endpts.hxx
static const uint8_t   ENDPOINTS[NUM_IN_OUT_ENDPOINTS] = {7, 2, 13, 9, 1, 15, 5};



const char* const speed_names[] = {"illegal speed code value",
                                   "1.5 Mbit/s (USB LowSpeed)",
                                   "12 Mbit/s (USB FullSpeed)",
                                   "480 Mbit/s (USB HighSpeed)",
                                   "5000 Mbit/s (USB SuperSpeed)",
                                   "10000 Mbit/s (USB SuperSpeedPlus)",
                                   "illegal speed code value",
                                  };

libusb_device_handle    *device_handle         ;
uint8_t                 echo[MAX_IN_PACKET + 1];  // terminating '\0'

extern "C" void close(
int     signum)
{
    if (signum != SIGINT)
        return;

    std::cout << "\nclose (signal "
              << signum
              << "):"
              << std::endl;

    std::cout << "releasing interface ..." << std::endl;
    libusb_release_interface(device_handle, 0);

    std::cout << "closing device handle ..." << std::endl;
    libusb_close(device_handle);

    std::cout << "exiting" << std::endl;

    exit(signum);
}

}  // namespace




int main(
int      argc  ,
char    *argv[])
{
    bool        use_many       = false ;
    uint8_t     vp_ndx         = 1     ,
                pended         = 7     ; // prepended "cccc+ " + appended '\n'

    uint16_t    max_in_packet  = MAX_IN_PACKET,
                max_out_packet = max_in_packet,
                vendor         = VENDOR       ,
                product        = PRODUCT      ;

    if (argc < 2 || argv[1][0] != '-') {
        std::cerr << "Usage: "
                  << argv[0]
                  << " -s|-m [vid pid]\n"
                  << "-s        use with usb_echo_simple.elf\n"
                  << "-m        use with usb_echo_max_endpts.elf\n"
                  << "vid pid   vendor id, product id"
                  << std::endl;
        return 1;
    }

    if (argv[1][1] == 'm') {
        use_many       = true;
        max_in_packet  = 16  ;  // see usb_dev_max_endpts.hxx
        max_out_packet = 16  ;  //  "          "         . "
        pended         =  9  ;  // prepended "e cccc+ " + appended '\n'
        vp_ndx         =  2  ;
    }

    uint16_t    max_expect_packet =   max_out_packet + pended < max_in_packet
                                    ? max_out_packet + pended
                                    : max_in_packet                          ,
                max_echo_data     = max_in_packet - pended                   ;

    if (argc == vp_ndx + 2) {
        vendor  = strtol(argv[vp_ndx    ], 0, 16);
        product = strtol(argv[vp_ndx + 1], 0, 16);
    }

    int     error;

    if ((error = libusb_init(0)) != static_cast<int>(LIBUSB_SUCCESS)) {
        std::cerr << "libusb_init() failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        return error;
    }

    if (!(device_handle = libusb_open_device_with_vid_pid(0, vendor, product))){
        std::cerr << "libusb_open_device_with_vid_pid(0, "
                  << std::hex
                  << std::setw(4)
                  << std::setfill('0')
                  << vendor
                  << ", "
                  << product
                  << ") failure"
                  << std::endl;
        return 1;
    }

    if (   (error = libusb_set_auto_detach_kernel_driver(device_handle, 1))
        != static_cast<int>(LIBUSB_SUCCESS)                          ) {
        std::cerr << "libusb_set_auto_detach_kernel_driver(device_handle, 1) "
                     "failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        return error;
    }

    if (   (error = libusb_claim_interface(device_handle, 0))
        != static_cast<int>(LIBUSB_SUCCESS)                  ) {
        std::cerr << "libusb_claim_interface() failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        return error;
    }

    signal(SIGINT, close);

    uint8_t out_endpoint = OUT_ENDPOINT,
             in_endpoint =  IN_ENDPOINT;

    for (std::string input ; std::getline(std::cin, input) ; ) {
        size_t  length = input.length(),
                offset = 0             ;
        int     bytes_transferred      ;


        if (use_many) {
            out_endpoint = ENDPOINTS[rand() % NUM_IN_OUT_ENDPOINTS];
             in_endpoint = out_endpoint | 0x80;  // high bit, USB API
        }

         while (length) {
            uint8_t     *current =   reinterpret_cast<uint8_t*>(
                                              const_cast<char*>(
                                                   input.c_str()))
                                   + offset                       ;

            size_t    xfer_length
                    = std::min(length,
                               static_cast<size_t>(max_out_packet));

            if (   (error = libusb_interrupt_transfer(device_handle     ,
                                                      out_endpoint      ,
                                                      current           ,
                                                      xfer_length       ,
                                                      &bytes_transferred,
                                                      OUT_TIMEOUT       ))
                != static_cast<int>(LIBUSB_SUCCESS)                       ) {

                std::cerr << "libusb_interrupt_transfer(, "
                          << static_cast<unsigned>(out_endpoint)
                          << "(OUT_EP), "
                          << xfer_length
                          << ", <"
                          << bytes_transferred
                          << ">, "
                          << OUT_TIMEOUT
                          << ") failure: "
                          << libusb_strerror(static_cast<libusb_error>(error))
                          << '('
                          << error
                          << ')'
                          << std::endl;
                close(error);
            }

            unsigned    fulls  = xfer_length / max_echo_data,
                        extra  = xfer_length % max_echo_data,
                        expect =   fulls *  max_in_packet   ;

            if (extra) expect += extra + pended;

            bool    zero_length_packet = false;

            while (expect || zero_length_packet) {
                if (   (error = libusb_interrupt_transfer(device_handle     ,
                                                          in_endpoint       ,
                                                          echo              ,
                                                          max_in_packet     ,
                                                          &bytes_transferred,
                                                          IN_TIMEOUT        ))
                    != static_cast<int>(LIBUSB_SUCCESS)                 ) {

                    std::cerr << "libusb_interrupt_transfer(, "
                              << static_cast<unsigned>(in_endpoint)
                              << "(IN_EP), <"
                              << bytes_transferred
                              << ">, "
                              << IN_TIMEOUT
                              << ") failure: "
                              << libusb_strerror(static_cast<libusb_error>
                                                            (error)       )
                              << '('
                              << error
                              << ")  expect: "
                              << expect
                              << " bytes (in max "
                              << max_in_packet
                              << " sized transfers)"
                              << std::endl;
                    close(error);
                }

                echo[bytes_transferred] = '\0';
                std::cout << echo             ;

                if (zero_length_packet)
                    break;  // got it, done

                // can't be here if expect already zero but check anyway
                // errors can cause stuck data to be sent late/unexpectedly
                else if (     expect
                         && ((expect -= bytes_transferred) == 0)
                         && bytes_transferred == max_in_packet  )
                    zero_length_packet = true;
            }

            std::cout << std::endl;

            offset += xfer_length    ;
            length -= xfer_length    ;

        }  // while (length)

    }  // main loop

    close(SIGINT);

}  // main()
