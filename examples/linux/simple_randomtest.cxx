// papoon_usb: "Not Insane" USB library for STM32F103xx MCUs
// Copyright (C) 2019 Mark R. Rubin
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
#include <string.h>

#include <iomanip>
#include <iostream>

#include <libusb.h>

#include <random_test.hxx>


#if !defined(UP_MAX_SIZE) || !defined(DOWN_MAX_SIZE)
#error must define UP_MAX_SIZE and DOWN_MAX_SIZE
#endif

#ifndef SEND_B4_RECV_MAX
#error must define SEND_B4_RECV_MAX
#endif

#if    UP_MAX_SIZE !=   2 \
    && UP_MAX_SIZE !=   4 \
    && UP_MAX_SIZE !=   8 \
    && UP_MAX_SIZE !=   16 \
    && UP_MAX_SIZE !=   32 \
    && UP_MAX_SIZE !=   64 \
    && UP_MAX_SIZE !=  128 \
    && UP_MAX_SIZE !=  256 \
    && UP_MAX_SIZE !=  512 \
    && UP_MAX_SIZE != 1024
#error UP_MAX_SIZE must be even power of 2
#endif

#if DOWN_MAX_SIZE % 4 != 0
#error DOWN_MAX_SIZE must be evenly divisible by 4
#endif

#if !defined(UP_SEED) || !defined(DOWN_SEED)
#error must define UP_SEED and DOWN_SEED
#endif

#if !defined(MASTER_LENGTH_SEED) || !defined(SLAVE_LENGTH_SEED)
#error must define MASTER_LENGTH_SEED and SLAVE_LENGTH_SEED
#endif

#ifndef SYNC_LENGTH
#error must define SYNC_LENGTH
#endif

#ifndef HISTOGRAM_LENGTH
#error must define HISTOGRAM_LENGTH
#endif

#ifndef REPORT_EVERY
#error must define REPORT_EVERY
#endif



namespace {

enum class TransferType {
    INTERRUPT = 0,
    BULK
};


static const uint16_t   VENDOR         = 0x0483,
                        PRODUCT        = 0x62e3,
                        MAX_IN_PACKET  = 64    ,
                        MAX_OUT_PACKET = 64    ;

static const int        OUT_TIMEOUT = 10000,     // milliseconds (10 seconds)
                         IN_TIMEOUT =     0;     // no timeout

static const uint8_t     IN_ENDPOINT = 0x81,
                        OUT_ENDPOINT = 0x02;

libusb_device_handle                    *device_handle = 0;
libusb_transfer                         *transfer_in   = 0,
                                        *transfer_out  = 0;

unittest::RandomTest<HISTOGRAM_LENGTH>   random_test(UP_SEED           ,
                                                     DOWN_SEED         ,
                                                     MASTER_LENGTH_SEED,
                                                     DOWN_MAX_SIZE     ,
                                                     SEND_B4_RECV_MAX  );

struct timeval  prev                                   ;
uint8_t         down_buf[DOWN_MAX_SIZE]                ,
                  up_buf[  UP_MAX_SIZE]                ;
struct timeval  begin = {0}                            ;
unsigned        handle_events = 0                      ,
                sends_pending = 0                      ,
                  up_errors   = 0                      ,
                down_errors   = 0                      ;
TransferType    transfer_type = TransferType::INTERRUPT;
bool            down_pending  = false                  ;
uint8_t         in_endpoint   =  IN_ENDPOINT           ,
                out_endpoint  = OUT_ENDPOINT           ;

unsigned    ups   = 0,   // DEBUG
            sends = 0,   // DEBUG
            downs = 0;   // DEBUG

unsigned         out_timeouts = 0,
            last_out_timeout  = 0;    // DEBUG



void report(
int     signum)
{
    std::cout << "\nreport (signal "
              << signum
              << "):"
              << std::endl;

    struct timeval  crnt, diff;

    if (gettimeofday(&crnt, 0) == -1) {
        std::cerr << "gettimeofday() crnt failure:"
                  << strerror(errno)
                  << " ("
                  << errno
                  << ')'
                  << std::endl;
        report(-1);
    }

    timersub(&crnt, &begin, &diff);

    double    elapsed
            =   static_cast<double>(diff.tv_sec )
              + static_cast<double>(diff.tv_usec) / 1000000.0,
              bytes_per_packet
            =   random_test.send_count()
              ?   static_cast<double>(random_test.send_bytes())
                / static_cast<double>(random_test.send_count())
              : 0.0                                            ;

    std::cout << "evnt: "
              << handle_events
              << "  sends_pending: "
              << sends_pending
              << "\nerrs: "
              << up_errors
              << " up  "
              << down_errors
              << " down"
              << std::endl;

    std::cout << "outs: "
              << out_timeouts
              << " timouts, last @ "
              << last_out_timeout
              << std::endl;

    std::cout << "dbug: "// DEBUG
              << sends
              << " sends  "
              << downs
              << " downs  "
              << ups
              << " ups"
              << std::endl;

    std::cout << "send: "
              << random_test.send_bytes()
              << " bytes  "
              << random_test.send_count()
              << " xfers  "
              << elapsed
              << " seconds\navgs: "
              << std::fixed
              << std::setprecision(1)
              << static_cast<double>(random_test.send_count()) / elapsed
              << " xfers/sec  "
              << static_cast<double>(random_test.send_bytes()) / elapsed
              << " bytes/sec  "
              << std::fixed
              << std::setprecision(2)
              << bytes_per_packet
              << " bytes/xfer\nrecvs_btwn histogram:";

    for (unsigned ndx = 0 ; ndx < random_test.histogram_length() ; ++ndx)
        std::cout << ' '
                  << random_test.recvs_btwn_sends(ndx) ;
    std::cout << std::endl;

      bytes_per_packet
    =   random_test.recv_count()
      ?   static_cast<double>(random_test.recv_bytes())
        / static_cast<double>(random_test.recv_count())
      : 0.0                                            ;

    std::cout << "recv: "
              << random_test.recv_bytes()
              << " bytes  "
              << random_test.recv_count()
              << " xfers  "
              << elapsed
              << " seconds\navgs: "
              << std::fixed
              << std::setprecision(1)
              << static_cast<double>(random_test.recv_count()) / elapsed
              << " xfers/sec  "
              << static_cast<double>(random_test.recv_bytes()) / elapsed
              << " bytes/sec  "
              << std::fixed
              << std::setprecision(2)
              << bytes_per_packet
              << " bytes/xfer\nsends_btwn histogram:";

    for (unsigned ndx = 0 ; ndx < random_test.histogram_length() ; ++ndx)
        std::cout << ' '
                  << random_test.sends_btwn_recvs(ndx) ;
    std::cout << std::endl;

    if (signum != 0) {
        libusb_release_interface(device_handle, 0);
        libusb_close(device_handle);
        exit(signum);
    }
}


void report(
int         signum ,
unsigned    linenum)
{
    std::cerr << "Abort, line "
              << linenum
              << "  code: "
              << signum;

    report(signum);
}




#ifdef DEBUG
uint16_t checksum(
const uint8_t* const    buffer,
const unsigned          length)
{
    uint16_t     sum = 0;

    for (unsigned ndx = 0 ; ndx < length ; ++ndx)
        sum += buffer[ndx];

    return sum;
}

std::string buffer_ascii(
const uint8_t* const    buffer,
const unsigned          length)
{
    std::stringstream   stream;

    for (unsigned ndx = 0 ; ndx < length ; ++ndx)
        stream << (  isprint(buffer[ndx])
                   ? static_cast<char>(buffer[ndx])
                   : '.');

    return stream.str();
}
#endif



void up_callback(
struct libusb_transfer  *transfer)
{
    ++ups;   // DEBUG

    if (   transfer->status != LIBUSB_TRANSFER_COMPLETED
        || transfer->actual_length <= 0) {
        std::cerr << "up_callback() transfer error: status: 0x"
                  << std::hex
                  << transfer->status
                  << std::dec
                  << "  length: "
                  << transfer->length
                  << "  actual_length="
                  << transfer->actual_length
                  << std::endl;
        ++up_errors;

        report(-1, __LINE__);
    }

#ifdef DEBUG
    std::cerr << "read "
              << std::setw(2)
              << transfer->actual_length
              << " bytes  cksum: 0x"
              << std::hex
              << std::setw(4)
              << std::setfill('0')
              << checksum(up_buf, transfer->length)
              << std::setfill(' ')
              << std::dec
              << "  "
              << buffer_ascii(up_buf, transfer->length)
              << std::endl;
#endif

    if (random_test.synced()) {
        if (!random_test.recv(up_buf, transfer->actual_length)) {
            std::cerr << "Data error in up packet"
                      << std::endl;
            report(-1, __LINE__);
        }
    }
    else
        random_test.recv_sync(up_buf, transfer->actual_length);

#ifdef RANDOMTEST_LIBUSB_ASYNC
    RESUBMIT:
#if 0  // should stay as previously set, don't need to redo
    if (transfer_type == TransferType::INTERRUPT)
        libusb_fill_interrupt_transfer(transfer_in  ,
                                       device_handle,
                                       in_endpoint  ,
                                       up_buf       ,
                                       MAX_IN_PACKET,
                                       up_callback  ,
                                       0            ,
                                       IN_TIMOUT    );
    else
        libusb_fill_bulk_transfer     (transfer_in  ,
                                       device_handle,
                                       in_endpoint  ,
                                       up_buf       ,
                                       MAX_IN_PACKET,
                                       up_callback  ,
                                       0            ,
                                       IN_TIMOUT    );
#endif

    int     error;
    if (   (error = libusb_submit_transfer(transfer))
        != static_cast<int>(LIBUSB_SUCCESS)              ) {
        std::cerr << "libusb_submit_transfer(<in_transfer, callback()>) "
                     "failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        report(-1, __LINE__) ;
    }
#endif
}


#ifdef RANDOMTEST_LIBUSB_ASYNC
void down_callback(
struct libusb_transfer  *transfer)
{
    ++downs;   // DEBUG

    if (transfer->status == LIBUSB_TRANSFER_TIMED_OUT) {
           ++out_timeouts;
        last_out_timeout = random_test.send_count();

        // need to resubmit?
        return;
    }

    if (   transfer->status != LIBUSB_TRANSFER_COMPLETED
        || transfer->actual_length != transfer->length  ) {
        std::cerr << "down_callback() transfer error: status: 0x"
                  << std::hex
                  << transfer->status
                  << std::dec
                  << "  length: "
                  << transfer->length
                  << "  actual_length="
                  << transfer->actual_length
                  << std::endl;
        ++down_errors;
        int     error;

        if (   (error = libusb_submit_transfer(transfer))
            != static_cast<int>(LIBUSB_SUCCESS)              ) {
            std::cerr << "libusb_submit_transfer(<in_transfer, callback()>) "
                         "failure: "
                      << libusb_strerror(static_cast<libusb_error>(error))
                      << '('
                      << error
                      << ')'
                      << std::endl;
            report(-1, __LINE__) ;
        }
    }

    down_pending = false;

#ifdef DEBUG
    std::cerr << "wrtn "
              << std::setw(2)
              << transfer->actual_length
              << " bytes cksum: 0x"
              << std::hex
              << std::setw(4)
              << std::setfill('0')
              << checksum(down_buf, transfer->actual_length)
              << std::setfill(' ')
              << std::dec
              << "  "
              << buffer_ascii(down_buf, transfer->actual_length)
              << std::endl;
#endif
}
#endif   // ifdef RANDOMTEST_LIBUSB_ASYNC



void send(
const uint16_t  length)
{
    if (down_pending)   // DEBUG
        std::cerr << "send() impossible down_pending==true"
                  << std::endl;

    ++sends;   // DEBUG
    int     error;
#ifdef RANDOMTEST_LIBUSB_ASYNC
    if (transfer_type == TransferType::INTERRUPT)
            libusb_fill_interrupt_transfer(transfer_out ,
                                           device_handle,
                                           out_endpoint ,
                                           down_buf     ,
                                           length       ,
                                           down_callback,
                                           0            ,
                                           OUT_TIMEOUT  );
    else
            libusb_fill_bulk_transfer     (transfer_out ,
                                           device_handle,
                                           out_endpoint ,
                                           down_buf     ,
                                           length       ,
                                           down_callback,
                                           0            ,
                                           OUT_TIMEOUT  );

    down_pending = true;

    if (   (error = libusb_submit_transfer(transfer_out))
        != static_cast<int>(LIBUSB_SUCCESS)              ) {
        std::cerr << "libusb_submit_transfer(<transfer_out, send()>) failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        report(error, __LINE__);
    }
#else
    int     bytes_transferred;
    if (transfer_type == TransferType::INTERRUPT)
        error = libusb_interrupt_transfer(device_handle     ,
                                          out_endpoint      ,
                                          down_buf          ,
                                          length            ,
                                          &bytes_transferred,
                                          OUT_TIMEOUT       );
    else
        error = libusb_bulk_transfer     (device_handle     ,
                                          out_endpoint      ,
                                          down_buf          ,
                                          length            ,
                                          &bytes_transferred,
                                          OUT_TIMEOUT       );

    if (   error             != static_cast<int>(LIBUSB_SUCCESS)
        || bytes_transferred != length                          ) {
        std::cerr << "out libusb_interrupt_transfer() failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        report(-1, __LINE__);
    }

#ifdef DEBUG
    std::cerr << "writ "
              << std::setw(2)
              << bytes_transferred
              << " bytes cksum: 0x"
              << std::hex
              << std::setw(4)
              << std::setfill('0')
              << checksum(down_buf, bytes_transferred)
              << std::setfill(' ')
              << std::dec
              << "  "
              << buffer_ascii(down_buf, bytes_transferred)
              << std::endl;
#endif
#endif
}

}  // namespace



int main(
int      argc  ,
char    *argv[])
{
    uint16_t    vendor  = VENDOR ,
                product = PRODUCT;

    if (argc > 1 && argv[1][0] == '-') {
        std::cout << "Usage: "
                  << argv[0]
                  << " [<vendor id> [product id [in endpoint [out endpoint "
                     " [i<nterupt>|b<ulk>]]]]]"
                  << std::endl;
        return 0;
    }

    if (argc > 1) vendor        = strtol(argv[1], 0, 16);
    if (argc > 2) product       = strtol(argv[2], 0, 16);
    if (argc > 3)  in_endpoint  = strtol(argv[3], 0, 16);
    if (argc > 4) out_endpoint  = strtol(argv[4], 0, 16);
    if (argc > 4) out_endpoint  = strtol(argv[4], 0, 16);
    if (argc > 5) transfer_type =   argv[5][0] == 'i'
                                  ? TransferType::INTERRUPT
                                  : TransferType::BULK     ;

    int  error;
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

#if 0  // not necessary, Linux kernel has no driver for custom simple USB device
    if (   (error = libusb_set_auto_detach_kernel_driver(device_handle, 1))
        != static_cast<int>(LIBUSB_SUCCESS)                          ) {
        std::cerr << "libusb_set_auto_detach_kernel_driver(device_handle, 1)) "
                     "failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        return error;
    }
#endif

    if (   (error = libusb_claim_interface(device_handle, 0))
        != static_cast<int>(LIBUSB_SUCCESS)                          ) {
        std::cerr << "libusb_claim_interface(device_handle, 0) failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        return error;
    }

    if (!(transfer_in = libusb_alloc_transfer(0))) {
        std::cerr << "libusb_alloc_transfer() for transfer_in failure"
                  << std::endl;
        return 1;
    }

    if (!(transfer_out = libusb_alloc_transfer(0))) {
        std::cerr << "libusb_alloc_transfer() for transfer_out failure"
                  << std::endl;
        return 1;
    }

    libusb_set_auto_detach_kernel_driver(device_handle, 1);

    if (gettimeofday(&prev, 0) == -1) {
        std::cerr << "gettimeofday() first call failure:"
                  << strerror(errno)
                  << " ("
                  << errno
                  << ')'
                  << std::endl;
        return 1;
    }

    signal(SIGINT, report);

#ifdef RANDOMTEST_LIBUSB_ASYNC
    if (transfer_type == TransferType::INTERRUPT)
        libusb_fill_interrupt_transfer(transfer_in  ,
                                       device_handle,
                                       in_endpoint  ,
                                       up_buf       ,
                                       MAX_IN_PACKET,
                                       up_callback  ,
                                       0            ,
                                       IN_TIMEOUT   );
    else
        libusb_fill_bulk_transfer     (transfer_in  ,
                                       device_handle,
                                       in_endpoint  ,
                                       up_buf       ,
                                       MAX_IN_PACKET,
                                       up_callback  ,
                                       0            ,
                                       IN_TIMEOUT   );

    if (   (error = libusb_submit_transfer(transfer_in))
        != static_cast<int>(LIBUSB_SUCCESS)              ) {
        std::cerr << "libusb_submit_transfer(<transfer_in, main()>) failure: "
                  << libusb_strerror(static_cast<libusb_error>(error))
                  << '('
                  << error
                  << ')'
                  << std::endl;
        return error;
    }
#endif

    random_test.send_sync(down_buf, SYNC_LENGTH);
    send(SYNC_LENGTH);


    struct timeval      tv            = {20, 0};    // 20 seconds
    unsigned            prev_report   = 0      ;

    while (true) {
#ifdef RANDOMTEST_LIBUSB_ASYNC
        if (   (error = libusb_handle_events_timeout_completed(0, &tv, 0))
            != static_cast<int>(LIBUSB_SUCCESS)                       ) {
            std::cerr << "libusb_handle_events_completed() failure: "
                      << libusb_strerror(static_cast<libusb_error>(error))
                      << '('
                      << error
                      << ')'
                      << std::endl;
            report(-1, __LINE__);
        }

        ++handle_events;
#endif   // ifdef RANDOMTEST_LIBUSB_ASYNC

        if (begin.tv_sec == 0 && random_test.synced()) {
            if (gettimeofday(&begin, 0) == -1) {
                std::cerr << "gettimeofday() begin failure:"
                          << strerror(errno)
                          << " ("
                          << errno
                          << ')'
                          << std::endl;
                report(-1, __LINE__);
            }
        }
        else if (     random_test.send_count()
                    + random_test.recv_count()
                    - prev_report
                 >= REPORT_EVERY              ) {
            report(0);
            prev_report = random_test.send_count() + random_test.recv_count();
        }


        struct timeval  crnt, diff;

        if (gettimeofday(&crnt, 0) == -1) {
            std::cerr << "gettimeofday() crnt failure:"
                      << strerror(errno)
                      << " ("
                      << errno
                      << ')'
                      << std::endl;
            report(-1, __LINE__);
        }

        timersub(&crnt, &prev, &diff);
#ifdef DEBUG
        if (diff.tv_sec < 1) continue;
#else
        if (!random_test.synced() && diff.tv_sec < 1) continue;
#endif
        prev = crnt;

#ifndef RANDOMTEST_LIBUSB_ASYNC
        int     bytes_transferred;
        if (transfer_type == TransferType::INTERRUPT)
            error = libusb_interrupt_transfer(device_handle     ,
                                              in_endpoint       ,
                                              up_buf            ,
                                              MAX_IN_PACKET     ,
                                              &bytes_transferred,
                                              IN_TIMEOUT        );
        else
            error = libusb_bulk_transfer     (device_handle     ,
                                              in_endpoint       ,
                                              up_buf            ,
                                              MAX_IN_PACKET     ,
                                              &bytes_transferred,
                                              IN_TIMEOUT        );

        if (error != static_cast<int>(LIBUSB_SUCCESS)) {
            std::cerr << "libusb_{interrupt,bulk}_transfer() failure: "
                      << libusb_strerror(static_cast<libusb_error>(error))
                      << '('
                      << error
                      << ')'
                      << std::endl;

            report(error, __LINE__);
        }

        transfer_in->actual_length = bytes_transferred;
        up_callback(transfer_in);
#endif

#ifdef RANDOMTEST_LIBUSB_ASYNC
        if (down_pending)
            ++sends_pending;
        else {
#endif
            uint16_t    length = random_test.send(down_buf);

            if (length)  // will be zero if not yet synced
                send(length);
#ifdef RANDOMTEST_LIBUSB_ASYNC
        }
#endif
    }

}  // main()
