# papoon_usb: "Not Insane" USB library for STM32F103xx MCUs
# Copyright (C) 2019 Mark R. Rubin
#
# This file is part of papoon_usb.
#
# The regbits_stm program is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# The regbits_stm program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# (LICENSE.txt) along with the regbits_stm program.  If not, see
# <https:#www.gnu.org/licenses/gpl.html>


#!/usr/bin/env python2

import fcntl
import os
import select
import signal
import string
import sys

tty = '/dev/ttyACM0'
PER = 16
MAX = 1024



def signal_handler(sig, frame):
    sys.stdout.write('\n')
    sys.exit(0)

if len(sys.argv) > 1:
    if   sys.argv[1].startswith('/dev/tty'): tty =              sys.argv[1]
    elif sys.argv[1].startswith(     'tty'): tty = '/dev/'    + sys.argv[1]
    else:                                    tty = '/dev/tty' + sys.argv[1]

signal.signal(signal.SIGINT, signal_handler)

read = open(tty, 'r')
writ = open(tty, 'w')

curflags = fcntl.fcntl(read.fileno(), fcntl.F_GETFL)
fcntl.fcntl(read.fileno(), fcntl.F_SETFL, curflags | os.O_NONBLOCK)

while True:
    (readers, writers, exceptors) = select.select((sys.stdin, read), (), ())
    if read in readers:
        input = read.read(MAX)
        sys.stdout.write("".join([     letter
                                  if   letter in string.printable
                                  else '.'
                                  for  letter
                                  in   input                      ]))
        if any([c not in string.printable for c in input]):
            sys.stdout.write('\n')
            ords = [     " " + letter + " "
                    if   letter in string.printable
                    else "%02x " % ord(letter)
                    for  letter
                    in   input               ]
            for begin in range(0, len(ords), PER):
                sys.stdout.write("%s\n" % "".join(ords[begin:begin+PER]))
            sys.stdout.write('\n')
        sys.stdout.flush()
    if sys.stdin in readers:
        input = sys.stdin.readline()
        # sys.stdout.write(input)
        writ.write(input)
