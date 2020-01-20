# papoon_usb: "Not Insane" USB library for STM32F103xx MCUs
# Copyright (C) 2019,2020 Mark R. Rubin
#
# This file is part of papoon_usb.
#
# The papoon_usb program is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# The papoon_usb program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# (LICENSE.txt) along with the papoon_usb program.  If not, see
# <https:#www.gnu.org/licenses/gpl.html>


PROGRAMS = stdin simple_randomtest tty_randomtest

DEBUG           ?= -U
EXTRA_CXX_FLAGS ?=
SEND_B4_RECV    ?= 64
REPORT_EVERY    ?= 10000
ASYNC		?= -D

SYNC_LEN  ?= 4
LNTH_SEED ?= 0x769bc5e6

CONFIGURATION = -DUP_MAX_SIZE=64			\
		-DDOWN_MAX_SIZE=64			\
		-DSEND_B4_RECV_MAX=$(SEND_B4_RECV)	\
		-DUP_SEED=0x5f443bba			\
		-DDOWN_SEED=0x684053d8			\
		-DMASTER_LENGTH_SEED=0x769bc5e6		\
		-DSLAVE_LENGTH_SEED=0x4b6420e0		\
		-DSYNC_LENGTH=$(SYNC_LEN) 		\
		-DHISTOGRAM_LENGTH=8			\
		-DREPORT_EVERY=$(REPORT_EVERY)		\
		$(ASYNC)RANDOMTEST_LIBUSB_ASYNC		\
		$(DEBUG)DEBUG

CXX_FLAGS = -g $(EXTRA_CXX_FLAGS)

INCLUDES     = -I/usr/include/libusb-1.0 -I../../util
LIBS         = -lusb-1.0

all: $(PROGRAMS)

stdin: stdin.o
	$(CXX) $^ $(LIBS) -o $@
simple_randomtest: simple_randomtest.o
	$(CXX) $^ $(LIBS) -o $@
tty_randomtest: tty_randomtest.o
	$(CXX) $^ $(LIBS) -o $@


.PHONY: clean
clean:
	rm -f $(PROGRAMS) *.o

%.o: %.cxx
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE_DIRS) $(INCLUDES) $(CONFIGURATION) \
               $<  -o $@
