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


#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <string>
#ifdef DEBUG
#include <linux/limits.h>
#include <sstream>
#endif

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



#define DEFAULT_DEV_TTY     "/dev/ttyACM0"


unittest::RandomTest<HISTOGRAM_LENGTH>  random_test(UP_SEED           ,
                                                    DOWN_SEED         ,
                                                    MASTER_LENGTH_SEED,
                                                    DOWN_MAX_SIZE     ,
                                                    SEND_B4_RECV_MAX  );

uint8_t         down_buf[DOWN_MAX_SIZE],
                  up_buf[  UP_MAX_SIZE];
struct timeval  begin = {0}            ;



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

    if (signum != 0)
        exit(signum);
}



void send(
const int           tty_fd  ,
const uint16_t      length  ,
const std::string&  tty_name)
{
#ifdef DEBUG
    std::cerr << "writ "
              << std::setw(2)
              << length
              << " bytes to   "
              << tty_fd
              << " "
              << tty_name
              << "  cksum: 0x"
              << std::hex
              << std::setw(4)
              << std::setfill('0')
              << checksum(down_buf, length)
              << std::setfill(' ')
              << std::dec
              << "  "
              << buffer_ascii(down_buf, length)
              << std::endl;
#endif

    int  written;
    if ((written = write(tty_fd, down_buf, length)) < length) {
        std::cerr << "Error or short write to file descriptor "
                  << tty_fd
                  << " ("
                  << tty_name
                  << ")  "
                  << written
                  << " bytes of "
                  << length
                  << "  "
                  << strerror(errno)
                  << " ("
                  << errno
                  << ')'
                  << std::endl;
        report(-1);
    }
}


int main(
int      argc  ,
char    *argv[])
{
    std::string     tty_name(DEFAULT_DEV_TTY);
    int             tty_fd                   ;

#ifdef PTY_TEST
    if (argc < 2) {
        std::cerr << "Usage: "
                  << argv[0]
                  << " 'ptmx'|</dev/pts number>"
                  << std::endl;
        return 1;
    }

    tty_name = argv[1];
#else
    if (argc > 1) {
        if (tty_name.substr(0, 8) == "/dev/tty")
            ;

        else if (tty_name.substr(0, 3) == "tty")
            tty_name = std::string("/dev/") + tty_name;

        else
            tty_name = std::string("/dev/tty") + tty_name;
    }
#endif


#ifdef PTY_TEST
    if (tty_name == "ptmx")
        tty_name = "/dev/ptmx";

    if (tty_name == "/dev/ptmx") {
        if ((tty_fd = posix_openpt(O_RDWR | O_NOCTTY)) == -1) {
            std::cerr << "posix_openpt(O_RDWR) failure: "
                      << strerror(errno)
                      << " ("
                      << errno
                      << ')'
                      << std::endl;
            return 1;
        }

        if (grantpt(tty_fd) == -1) {
            std::cerr << "grantpt("
                      << tty_fd
                      << ") failure: "
                      << strerror(errno)
                      << " ("
                      << errno
                      << ')'
                      << std::endl;
            return 1;
        }

        if (unlockpt(tty_fd) == -1) {
            std::cerr << "unlockpt("
                      << tty_fd
                      << ") failure: "
                      << strerror(errno)
                      << " ("
                      << errno
                      << ')'
                      << std::endl;
            return 1;
        }

        char    *pts_name = ptsname(tty_fd);

        if (!pts_name) {
            std::cerr << "ptsname("
                      << tty_fd
                      << ") failure: "
                      << strerror(errno)
                      << " ("
                      << errno
                      << ')'
                      << std::endl;
            return 1;
        }

        std::cout << pts_name
                  << std::endl;
    }

    else {
        if (tty_name.substr(0, 9) != "/dev/pts/")
            tty_name = std::string("/dev/pts/") + tty_name;

       if ((tty_fd = open(tty_name.c_str(), O_RDWR | O_NOCTTY)) == -1) {
           std::cerr << "Can't open "
                     << tty_name
                     << " O_RDWR: "
                     << strerror(errno)
                     << " ("
                     << errno
                     << ')'
                     << std::endl;
           return 1;
       }
    }

#else

    for ( tty_fd = -1                                               ;
         (tty_fd = open(tty_name.c_str(), O_RDWR | O_NOCTTY)) == -1 ;
                                                                     ) {
        std::cerr << "Can't open "
                  << tty_name
                  << " O_RDWR: "
                  << strerror(errno)
                  << " ("
                  << errno
                  << ')'
                  << std::endl;

        if (errno != ENOENT && errno != EACCES && errno != EPERM)
            return 1;

        sleep(2);
    }

    std::cout << tty_name.c_str()
              << " opened"
              << std::endl;

#endif // ifdef PTY_TEST


    struct termios  term_modes;
    cfmakeraw(&term_modes);
    tcsetattr(tty_fd, TCSANOW, &term_modes);

    signal(SIGINT, report);


    random_test.send_sync(down_buf, SYNC_LENGTH);
    send(tty_fd, SYNC_LENGTH, tty_name);


    struct timeval  prev;
    if (gettimeofday(&prev, 0) == -1) {
        std::cerr << "gettimeofday() first call failure:"
                  << strerror(errno)
                  << " ("
                  << errno
                  << ')'
                  << std::endl;
        return 1;
    }

    unsigned    prev_report = 0;


    while (true) {
        fd_set  ins,
                outs,
                errs;

        FD_ZERO(&ins );
        FD_ZERO(&outs);
        FD_ZERO(&errs);

        FD_SET(tty_fd, & ins);
        FD_SET(tty_fd, &outs);
        FD_SET(tty_fd, &errs);

        int     nfds = tty_fd + 1,
                select_result;

        if ((select_result = select(nfds, &ins, &outs, &errs, 0)) == -1) {
            std::cerr << "select() error: "
                      << strerror(errno)
                      << " ("
                      << errno
                      << ')'
                      << std::endl;
            report(-1);
        }

        if (begin.tv_sec == 0 && random_test.synced()) {
            if (gettimeofday(&begin, 0) == -1) {
                std::cerr << "gettimeofday() begin failure:"
                          << strerror(errno)
                          << " ("
                          << errno
                          << ')'
                          << std::endl;
                report(-1);
            }
        }
        else if (     random_test.send_count()
                    + random_test.recv_count()
                    - prev_report
                 >= REPORT_EVERY              ) {
            report(0);
            prev_report = random_test.send_count() + random_test.recv_count();
        }


        if (FD_ISSET(tty_fd, &errs)) {
            std::cerr << "select() error, file descriptor: "
                          << tty_fd
                          << " ("
                          << tty_name
                          << "): "
                          << strerror(errno)
                          << " ("
                          << errno
                          << ')'
                          << std::endl;
            report(-1);
        }

        if (FD_ISSET(tty_fd, &ins)) {
            ssize_t     length = read(tty_fd                                ,
                                      up_buf                                ,
                                      random_test.synced() ? UP_MAX_SIZE : 4);

            if (length == -1) {
                std::cerr << "Error reading from file descriptor "
                          << tty_fd
                          << " ("
                          << tty_name
                          << "): "
                          << strerror(errno)
                          << " ("
                          << errno
                          << ')'
                          << std::endl;
                report(-1);
            }

#ifdef DEBUG
            std::cerr << "read "
                      << std::setw(2)
                      << length
                      << " bytes from "
                      << tty_fd
                      << " "
                      << tty_name
                      << "  cksum: 0x"
                      << std::hex
                      << std::setw(4)
                      << std::setfill('0')
                      << checksum(up_buf, length)
                      << std::setfill(' ')
                      << std::dec
                      << "  "
                      << buffer_ascii(up_buf, length)
                      << std::endl;
#endif

            if (!length) continue;

            if (random_test.synced()) {
                if (!random_test.recv(up_buf, length)) {
                    std::cerr << "Data error in "
#ifdef PTY_TEST
                              << (tty_name == "/dev/ptmx" ? "up" : "down")
#else
                              << "up"
#endif
                              << " packet"
                              << std::endl;
                    report(-1);
                }
            }
            else
                random_test.recv_sync(up_buf, length);
        }


        else if (FD_ISSET(tty_fd, &outs)) {
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

            timersub(&crnt, &prev, &diff);
#ifdef DEBUG
            if (diff.tv_sec < 1) continue;
#else
            if (!random_test.synced() && diff.tv_sec < 1) continue;
#endif
            prev = crnt;

            uint16_t    length = random_test.send(down_buf);

            if (length)  // otherwise stall until receive so as to not flood
                send(tty_fd, length, tty_name);
        }
    }

}  // main()
