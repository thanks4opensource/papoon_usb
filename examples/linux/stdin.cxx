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


#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <string>

#include <libusb.h>


namespace {
static const uint16_t   VENDOR         = 0x0483,
                        PRODUCT        = 0x62e3,
                        MAX_IN_PACKET  = 64    ,
                        MAX_OUT_PACKET = 56    ;  // allow "0x0000 ..." echo
                                                  // plus align mod 4

static const uint8_t     IN_ENDPOINT = 0x81,
                        OUT_ENDPOINT = 0x02;

static const int        OUT_TIMEOUT = 1000,  // milliseconds? not documented
                                             // in libusb.h
                         IN_TIMEOUT = 1000;


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

#if 0
void in_callback(
struct libusb_transfer  *transfer)
{
}
#endif

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
    uint16_t    vendor  = VENDOR ,
                product = PRODUCT;

    if (argc == 3) {
        vendor  = strtol(argv[1], 0, 16);
        product = strtol(argv[2], 0, 16);
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

#if 0
    if (   (error = libusb_reset_device(device_handle))
        != static_cast<int>(LIBUSB_SUCCESS)                          ) {
        std::cerr << "libusb_reset_device() failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        return error;
    }
#endif

    signal(SIGINT, close);

    for (std::string input ; std::getline(std::cin, input) ; ) {
        size_t  length = input.length(),
                offset = 0             ;
        int     bytes_transferred      ;

         while (length) {
            uint8_t     *current =   reinterpret_cast<uint8_t*>(
                                              const_cast<char*>(
                                                   input.c_str()))
                                   + offset                       ;

            size_t    xfer_length
                    = std::min(length,
                               static_cast<size_t>(MAX_OUT_PACKET));

            if (   (error = libusb_interrupt_transfer(device_handle     ,
                                                      OUT_ENDPOINT      ,
                                                      current           ,
                                                      xfer_length       ,
                                                      &bytes_transferred,
                                                      OUT_TIMEOUT       ))
                != static_cast<int>(LIBUSB_SUCCESS)                       ) {

                std::cerr << "libusb_interrupt_transfer(, "
                          << static_cast<unsigned>(OUT_ENDPOINT)
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

            if (   (error = libusb_interrupt_transfer(device_handle     ,
                                                      IN_ENDPOINT       ,
                                                      echo              ,
                                                      MAX_IN_PACKET     ,
                                                      &bytes_transferred,
                                                      IN_TIMEOUT        ))
                != static_cast<int>(LIBUSB_SUCCESS)                 ) {

                std::cerr << "libusb_interrupt_transfer(, "
                          << static_cast<unsigned>(IN_ENDPOINT)
                          << "(IN_EP), <"
                          << bytes_transferred
                          << ">, "
                          << IN_TIMEOUT
                          << ") failure: "
                          << libusb_strerror(static_cast<libusb_error>(error))
                          << '('
                          << error
                          << ')'
                          << std::endl;
                close(error);
            }

            echo[bytes_transferred] = '\0';
            std::cout << echo             ;

            offset += xfer_length    ;
            length -= xfer_length    ;

            // need to get either 2nd part of echo and/or
            // zero-length transfer after max UP packet
            // also libusb generates own zero-length packet on max length
            // UP xfer in addition to one usb_echo.cxx:run sends but only
            // if max UP is last of multiple transferes
            bool    expecting =    (bytes_transferred - xfer_length < 9)
                                    // 9 == prepended "cccc mm " + appened '\n'
                                &&  bytes_transferred                   ,
                                    // edge case blank line in case allowed
                    max_in    = bytes_transferred == MAX_IN_PACKET      ;

            while (expecting || max_in) {
                if (   (error = libusb_interrupt_transfer(device_handle     ,
                                                          IN_ENDPOINT       ,
                                                          echo              ,
                                                          MAX_IN_PACKET     ,
                                                          &bytes_transferred,
                                                          IN_TIMEOUT        ))
                    != static_cast<int>(LIBUSB_SUCCESS)                       ){
                    std::cerr << "libusb_interrupt_transfer(, "
                              << static_cast<unsigned>(IN_ENDPOINT)
                              << "(IN_EP), <"
                              << bytes_transferred
                              << ">, "
                              << IN_TIMEOUT
                              << ") failure: "
                              << libusb_strerror(static_cast<libusb_error>
                                                            (error)       )
                              << '('
                              << error
                              << ')'
                              << std::endl;
                    close(error);
                }

                echo[bytes_transferred] = '\0';
                std::cout << echo;

                expecting = expecting && max_in;  // handle libusb-generated
                max_in    = false              ;
            }

            if (length == 0) std::cout << std::endl;
        }
    }

    close(SIGINT);

}
