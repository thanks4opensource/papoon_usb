PROGRAMS = example.elf \
	   usb_simple_echo.elf  \
           usb_cdc_acm_echo.elf \
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

INCLUDES    =  -I../../usb -I../../regbits -I../../util -I../../arm -I.
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

usb_cdc_acm_echo.elf: usb_cdc_acm_echo.o usb_dev.o usb_echo.o usb_dev.o usb_dev_cdc_acm.o usb_mcu_init.o
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
