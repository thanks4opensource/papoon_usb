# papoon_usb: "Not Insane" USB library for STM32F103xx MCUs
# Copyright (C) 2019,2020 Mark R. Rubin
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


PROGRAMS = example.elf \
	   usb_simple_echo.elf  \
	   usb_echo_max_endpts.elf  \
           usb_cdc_acm_echo.elf \
           usb_cdc_acm_echo_c.elf \
	   usb_simple_randomtest.elf \
	   usb_cdc_acm_randomtest.elf \
	   usb_mouse.elf \
	   usb_midi.elf

DEBUG           ?= -U
EXTRA_CXX_FLAGS ?=
SEND_B4_RECV    ?= 64
REPORT_EVERY    ?= 10000
ASYNC		?= -D

SYNC_LEN  ?= 4
LNTH_SEED ?= 0x769bc5e6

CONFIGURATION = -DUP_MAX_PACKET_SIZE=64			\
		-DDOWN_MAX_PACKET_SIZE=64		\
		-DSEND_B4_RECV_MAX=$(SEND_B4_RECV)	\
		-DUSB_RANDOMTEST_UP_SEED=0x5f443bba	\
		-DUSB_RANDOMTEST_DOWN_SEED=0x684053d8	\
		-DUSB_RANDOMTEST_LENGTH_SEED=0x769bc5e6	\
		-DSLAVE_LENGTH_SEED=0x4b6420e0		\
		-DUSB_RANDOMTEST_SYNC_LENGTH=$(SYNC_LEN)\
		-DHISTOGRAM_LENGTH=8			\
		-DREPORT_EVERY=$(REPORT_EVERY)		\
		$(ASYNC)RANDOMTEST_LIBUSB_ASYNC		\
		$(DEBUG)DEBUG

CXX_FLAGS = -g -O1 $(EXTRA_CXX_FLAGS)
CC_FLAGS  = -g -O1

INCLUDES    =  -I../../usb 	\
	       -I../../regbits	\
	       -I../../util	\
	       -I../../arm	\
	       -I../../st	\
	       -I.
vpath %.cxx      ../../usb

all: $(PROGRAMS) warning

.PHONY: warning
warning: 
	@echo
	@echo THESE BINARIES MUST BE BUILT USING AN EMBEDDED ARM DEVELOPMENT
	@echo TOOLCHAIN. THIS BUILD HAS BEEN MERELY TO CONFIRM THAT ALL
	@echo NECESSARY SOURCE FILES HAVE BEEN INCLUDED IN THE REPOSITORY.

example.elf: example.o usb_dev.o usb_dev.o usb_dev_cdc_acm.o usb_mcu_init.o
	$(CXX) $^ -o $@

usb_simple_echo.elf: usb_simple_echo.o usb_echo.o usb_dev.o usb_dev_simple.o usb_mcu_init.o
	$(CXX) $^ -o $@

usb_echo_max_endpts.elf: usb_echo_max_endpts.o usb_echo.o usb_dev.o usb_dev_max_endpts.o usb_mcu_init.o
	$(CXX) $^ -o $@

usb_cdc_acm_echo.elf: usb_cdc_acm_echo.o usb_dev.o usb_echo.o usb_dev.o usb_dev_cdc_acm.o usb_mcu_init.o
	$(CXX) $^ -o $@

usb_cdc_acm_echo_c.elf: usb_cdc_acm_echo_c.o usb_echo_c.o usb_mcu_init_c.o usb_dev_cdc_acm_c.o usb_dev_cdc_acm.o usb_dev.o
	$(CXX) $^ -o $@

usb_simple_randomtest.elf: usb_simple_randomtest.o usb_dev.o usb_dev_simple.o usb_randomtest.o usb_mcu_init.o
	$(CXX) $^ -o $@

usb_cdc_acm_randomtest.elf: usb_cdc_acm_randomtest.o usb_dev.o usb_dev_cdc_acm.o usb_randomtest.o usb_dev.o usb_mcu_init.o
	$(CXX) $^ -o $@

usb_mouse.elf: mouse.o usb_dev.o usb_dev.o usb_dev_hid.o usb_dev_hid_mouse.o usb_mcu_init.o
	$(CXX) $^ -o $@

usb_midi.elf: midi.o usb_dev.o usb_dev_midi.o usb_mcu_init.o
	$(CXX) $^ -o $@


.PHONY: clean
clean:
	rm -f $(PROGRAMS) *.o

%.o: %.cxx
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE_DIRS) $(INCLUDES) $(CONFIGURATION) \
               $<  -o $@

%.o: %.c
	$(CC) -c $(CC_FLAGS) $(INCLUDE_DIRS) $(INCLUDES) $(CONFIGURATION) \
               $<  -o $@
