papoon_usb: "Not Insane" USB library for STM32F103xx MCUs
=========================================================

<a name="description"></a>
**papoon_usb** is a lightweight, efficient, cleanly-designed library for the USB Device peripheral in STMF103xx (and similar) MCUs. Its compiled binary size is approximately half that of the bloated, indirection-filled, spaghetti-code USB libraries on GitHub from ST and others; its performance boost is likely  similar. Best of all, the code is "Not Insane" [(1)](#not_insane) --- or at least is as sane as reasonably possible given the craziness of the USB architecture and ST's hardware implementation of it.



<br> <a name="contents"></a>
Contents
--------

* [License](#license)
* [How to use papoon_usb](#how_to_use_papoon_usb)
    * [TL;DNR ("Too Long, Did Not Read")](#tldnr_too_long_did_not_read)
        * [Simple example](#simple_example)
        * [Code description](#code_description)
    * [NLE;WTRM ("Not Long Enough, Want To Read More")](#nle_want_to_read_more)
        * [Interrupts, polling, callbacks](#interrupts_polling_callbacks)
        * [USB class implementations](#usb_class_implementations)
* [Example client applications](#example_client_applications)
* [Implementation of papoon_usb](#implementation_of_papoon_usb)
    * [C++](#cplusplus)
    * [regbits](#regbits_github)
* [Motivation for papoon_usb](#motivation_for_papoon_usb)
* [Insanity](#insanity)
    * [USB standard](#usb_standard)
    * [ST hardware and documentation](#st_hardware_and_documentation)
    * [ST software](#st_software)
* [Further development](#further_development)



<br> <a name="license"></a>
License
-------

papoon_usb: "Not Insane" USB library for STM32F103xx MCUs for STM MCUs

Copyright (C) 2019,2020 Mark R. Rubin

This file is part of papoon_usb.

The papoon_usb program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

The papoon_usb program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the [GNU General Public License](LICENSE.txt) along with the papoon_usb program.  If not, see <https://www.gnu.org/licenses/gpl.html>




<br> <a name="how_to_use_papoon_usb"></a>
How to use papoon_usb
---------------------

<a name="tldnr_too_long_did_not_read"></a>
### TL;DNR ("Too Long, Did Not Read")

Simple example <a name="simple_example"></a> of client application, using USB CDC-ACM class (source available in [example.cxx](examples/blue_pill/example.cxx)):

    #include <core_cm3.hxx>
    #include <stm32f103xb.hxx>

    #include <usb_dev_cdc_acm.hxx>

    #include <usb_mcu_init.hxx>


    using namespace stm32f10_12357_xx;


    UsbDevCdcAcm    usb_dev;

    uint8_t         recv_buf[UsbDevCdcAcm::CDC_OUT_DATA_SIZE],
                    send_buf[UsbDevCdcAcm::CDC_IN_DATA_SIZE ];

    int main()
    {
        usb_dev.serial_number_init();

        usb_mcu_init();

        usb_dev.init();

        while (usb_dev.device_state() != UsbDev::DeviceState::CONFIGURED)
            usb_dev.poll();

        while (true) {
            usb_dev.poll();

            uint16_t    recv_len,
                        send_len;

            if (recv_len = usb_dev.recv(UsbDevCdcAcm::CDC_ENDPOINT_OUT, recv_buf)) {
                // process data received from host
            }

            // fill buffer and set send_len

            while (!usb_dev.send(UsbDevCdcAcm::CDC_ENDPOINT_IN, send_buf, send_len))
                usb_dev.poll();
        }
    }

That's it. Where's `usbd_desc.c` (which doesn't contain any USB descriptors),  `usbd_cdc_interface.c`, `stm32f1xx_it.c`, ad nauseum --- all of which other libraries think application client coders should be required to implement/modify? (See [ST software](#st_software), below.)  Sorry: They're not needed here.

<br>
<br>
Code <a name="code_description"></a> snippets from the example, with brief additional information following each:

<br>
<br>

    #include <core_cm3.hxx>
    #include <stm32f103xb.hxx>

`regbits` system for type-safe STM32F103 (and generic) direct "bare metal" register-level programming (see [regbits](#regbits_github), below)

<br>

    #include <usb_dev_cdc_acm.hxx>

USB CDC-ACM class (see [USB class implementations](#usb_class_implementations) below)

<br>

    using namespace stm32f10_12357_xx;

see [C++](#cplusplus), below

<br>

    UsbDevCdcAcm    usb_dev;

instantiate USB class (C++ derived class)  object

<br>

    uint8_t         recv_buf[UsbDevCdcAcm::CDC_OUT_DATA_SIZE],
                    send_buf[UsbDevCdcAcm::CDC_IN_DATA_SIZE ];

Data buffers. Note "IN" and "OUT" are host-centric, as per USB standard nomenclature.

<br>

    int main()
    {

The application.

<br>

        usb_dev.serial_number_init();

Optional, to use STM32F103xx "Unique Device ID". Must be done before MCU clock initialization --- see [C++](#cplusplus) and [STM hardware and documentation](#unique_id), below.

<br>

        usb_mcu_init();

Initialize CPU clocks, USB peripheral, etc. See 
[usb_mcu_init.cxx](usb/usb_mcu_init.cxx) for example implementation.

<br>

        usb_dev.init();

Initialize library (and USB class) (see [C++](#cplusplus), below).

<br>

        while (usb_dev.device_state() != UsbDev::DeviceState::CONFIGURED)
            usb_dev.poll();

Optional --- UsbDev::recv() and UsbDev::send() will return 0 and false respectively until USB enumeration of device by host has been completed, but this is explicitly waits until the status is known. Also see [Interrupts, polling, callbacks](#interrupts_polling_callbacks), below, regarding related timing and performance issues.

<br>

        while (true) {
            usb_dev.poll();

See [Interrupts, polling, callbacks](#interrupts_polling_callbacks), below.

<br>

            uint16_t    recv_len,
                        send_len;

            if (recv_len = usb_dev.recv(UsbDevCdcAcm::CDC_ENDPOINT_OUT, recv_buf)) {
                // process data received from host
            }
            
Non-blocking read.


<br>

            // fill buffer and set send_len

            while (!usb_dev.send(UsbDevCdcAcm::CDC_ENDPOINT_IN, send_buf, send_len))
                usb_dev.poll();

Wait for send completion. (See [Interrupts, polling, callbacks](#interrupts_polling_callbacks), below)

<br>

        }
    }

End main loop, end `main()` function.



<br> <a name="nle_want_to_read_more"></a>
### NLE;WTRM ("Not Long Enough, Want To Read More")

<a name="interrupts_polling_callbacks"></a>
#### Interrupts, polling, callbacks

papoon_usb can be used in any of four modes chosen from a 2x2 configuration matrix:

                                   endpoint queries      endpoint callbacks
                                 ------------------    --------------------
            polled            |       poll+query            poll+callback
            interrupt driven  |  interrupt+quey        interrupt+callback                 

These are controlled by defining (or not) one or both of two macros: `USB_DEV_INTERRUPT_DRIVEN` and `USB_DEV_ENDPOINT_CALLBACKS`.

In `USB_DEV_INTERRUPT_DRIVEN` mode, client code must implement an ARM NVIC interrupt handler:

    extern "C" void USB_LP_CAN1_RX0_IRQHandler()
    {
        usb_dev.interrupt_handler();
    }

and enable it via code such as:

        arm::nvic->iser.set(arm::NvicIrqn::USB_LP_CAN1_RX0);

(using C++ with `regbits`'s [core_cm3.hxx](arm/core_cm3.hxx)) or:

        NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

(using C with [core_cm3.h](arm/core_cm3.h)).

In non-`USB_DEV_INTERRUPT_DRIVEN` (i.e. polled) mode, client application code must call `UsbDev::poll()` (via an instantiated object, e.g. `usb_dev.poll()`). The frequency with which this must be done depends on the USB device class (implemented in the C++ class derived from `UsbDev`) being used. In general, once USB device enumeration has been completed there should be no particular timing requirements as papoon_usb configures the STM32F103xx USB peripheral to cause the host to wait (repeatedly attempting to transfer data until confirmed) until the "OUT" (standard USB host-centric nomenclature) data has been retrieved by the client application calling `UsbDev::recv()`. Likewise, client code can send "IN" data to the host at any time, checking the return value of `UsbDev::send()` to see if the previous send (if any) has completed and the new data has been successfully queued for transfer.

To the contrary, before and during enumeration `poll()` must be called at a very high rate due to the speed of the USB 2.0 "full speed" protocol. It is best to do this in as tight a loop as possible, optimally:

        while (usb_dev.device_state() != UsbDev::DeviceState::CONFIGURED)
            usb_dev.poll();

although it might be acceptable to do some minimal additional processing within the loop. Note that given the latency involved in ARM interrupts this is probably faster than relying on them, although interrupts are sufficiently fast for (most? all?) USB host controllers and their host driver software implementations.

If the `USB_DEV_ENDPOINT_CALLBACKS` macro is defined, papoon_usb will call any functions registered via `UsbDev::register_recv_callback()` and `UsbDev::register_send_callback()` when data has been received on the registered endpoint or it is available to send data, respectively. C++ coders note that these must be global or namespace-scope functions (see [C++](#cplusplus), below), not object instance methods (no `std::bind` available). Note that enabling both polling and callbacks is not particularly useful: `UsbDev::poll()` returns a `uint16_t` with bits set indicating "ready" endpoints (which can be extracted using the `UsbDev::poll_recv_ready()` and `UsbDev::poll_recv_ready()` convenience functions), and explicitly executing the appropriate code either inline or by calling a "callback" function in the application's main loop is as efficient as allowing `UsbDev` to call the callback implicitly. The choice is a matter of the application developer's taste.

Regardless the polled-vs-interrupt-driven and callbacks-vs-direct configuration chosen, all the above methods use papoon_usb's `UsbDev::send()` and `UsbDev::recv()` methods to marshall data between application code `uint8_t*` buffers and the internal STM32F103xx USB peripheral's "PMA" memory. Data copying is done via CPU or DMA, controlled by defining (or not) the `USB_DEV_DMA_PMA` compilation macro. Testing has shown little or no performance benefit from using DMA in this use-case (as opposed to memory-to-memory copies to normal memory) but the code and option to use it has been retained regardless(see [Further development](#further_development), below).

Overall performance can, however, be increased by applications directly accessing PMA memory, eliminating the buffer copying overhead. This could consist of the application directly generating data to send to the host in PMA memory, directly reading/parsing received data, or using the STM32F103xx DMA engine to transfer data between another peripheral and the USB PMA memory. A classic example of the latter would be implementing a bidirectional USB-to-serial hardware bridge using the papoon_usb and the STM32F103xx USART peripheral.

A number of `UsbDev` class methods are provided for these use-cases, including non-buffer-copying `send()` and `recv()` methods, `recv_lnth()` and `recv_done()` (for status checking), `read()` and `writ()` (single `uint16_t` data copies), and `send_buf()` and `recv_buf()` (for obtaining raw memory addresses). Note that extreme care must be used when using these --- memory overwrites will almost certainly cause fatal application crashes, and careful attention must be paid to `uint8_t`, `uint16_t`, and `uint32_t` memory alignment and endian-ness. See the documentation in [usb_dev.hxx](usb/usb_dev.hxx) for further descriptions and information.

Finally, if only the above direct-access mehods are being used and the normal buffer-copying `send()` and `recv()` methods are not needed, the `USB_DEV_NO_BUFFER_RECV_SEND` macro can be defined to prevent their compilation and save space in the compiled application binary.



<a name="usb_class_implementations"></a>
#### USB class implementations

It is very easy to implement USB device classes (C++ classes derived from UsbDev) in papoon_usb ... or at least as easy as is possible given the excessive complexity of device classes in the USB standards.

This repository contains sample implementations for the following USB device classes:

* CDC/ACM (Communication Device Class, Abstract Control Model)
* HID mouse (Human Interface Device Class, mouse)
* MIDI
* "simple" (a minimal custom USB device class)

See code implementing these in:
* [usb_dev_cdc_acm.cxx](usb/usb_dev_cdc_acm.cxx)
* [usb_dev_hid_mouse.cxx](usb/usb_dev_hid_mouse.cxx)
* [usb_dev_midi.cxx](usb/usb_dev_midi.cxx)
* [usb_dev_simple.cxx](usb/usb_dev_simple.cxx)

and corresponding `.hxx` files. Note that [usb_dev_hid_mouse.cxx](usb/usb_dev_hid_mouse.cxx) is derived from an intermediate `UsbDevHid` class in [usb_dev_hid.hxx](usb/usb_dev_hid.hxx) and [usb_dev_hid.cxx](usb/usb_dev_hid.cxx) for future use in implementing e.g. an HID keyboard class.

A USB device class is implemented by deriving a new C++ class from the `UsbDev` base class, defining several `UsbDev` class methods and member variables (see [static polymorphism](#static_polymorphism), below), plus any USB class-specific member variables.  These include:

* The standard-required USB device descriptor `const uint8_t UsbDev::_DEVICE_DESC[]`
* The standard-required USB configuration descriptor `const uint8_t UsbDev::_CONFIG_DESC[]`, including all necessary sub-descriptors. Note that the `bLength` field needs to be set at runtime by the class' `init()` method.
* A `const uint8_t *UsbDev::_STRING_DESCS[]` array. This should include at minimum the `UsbDev::_LANGUAGE_ID_STRING_DESC[]`, accessed via `UsbDev::  language_id_string_desc()`, plus any other string descriptors referenced by index in the `_DEVICE_DESC` and/or other descriptors.
* A `void DerivedClass::init()` method  (which overrides `UsbDev::init()`) and must, at minimum, set the `UsbDev::CONFIG_DESC_SIZE_NDX` element of the `_CONFIG_DESC` to its correct value (i.e. `sizeof(_CONFIG_DESC)`) and call the base class' `UsbDev::init()`. Note that although common to all `UsbDev`-derived classes, this `sizeof()` initialization cannot be done in `UsbDev::init()` due to source file scope issues (see [static polymorphism](#static_polymorphism), below).
* Implement the base class `bool UsbDev::device_class_setup()` method. This method only needs to handle USB class-specific "setup" requests, accessed via the base class' `UsbDev::SetupPacket* _setup_packet` member object. The method should return `true` is it has actually executed any `setup` request(s), otherwise `false`, but needs to be implemented (returning a default value of `false`) regardless.
* `void UsbDev::set_configuration()` and `void UsbDev::set_interface()` which perform USB class-specific actions if required.

Again, see the provided example USB class implementation files for use as templates for creating a new USB class.



<br> <a name="example_client_applications"></a>
Example client applications
---------------------------

Several example client applications are provided as templates for using papoon_usb in the [examples/blue_pill](examples/blue_pill) directory. As the name suggests, the are written for the ubiquitous "Blue Pill" STM32F103xx development board which has an application-controlled LED connected to the MCU's GPIO PA13 port (active low). Code driving the LED, along with the GPIO initialization in [usb_mcu_init.cxx](usb/usb_mcu_init.cxx), can be easily removed or modified for other hardware environments.

The examples use USB classes (C++ classes derived from `UsbDev`) found in the [usb](usb) directory. Some examples work in conjunction with host software found in the [examples/linux](examples/linux) directory. This software variously relies on `libusb` ([project site](https://libusb.info/), [github](https://github.com/libusb/libusb)), or the Linux USB CDC-ACM driver <a name="linux_usb_cdc_acm_driver"></a>  which creates a `/dev/ttyACM0` *NIX device special file for reading/writing data. The latter may require using other OS-specific mechanisms. `libusb` is fairly OS-agnostic.

Makefiles ([linux](examples/linux/Makefile), [blue_pill](examples/linux/Makefile)) are provided. Note that the latter is a stub only (written to test this repository's completeness) and needs to be replaced with something matching the user's own ARM/STM embedded development toolchain environment.

The examples include:


##### CDC-ACM echo
STM32F103 USB device echoes back text sent to it via the USB CDC-ACM class protocol (see [USB insanity](#usb_standard), below). Text is prepended with a four-character hexadecimal sequence number, and a two-character hex index showing when a single DOWN (USB host-centric nomenclature) transfer has been split into two UP echoes due to the additional 8 bytes of sequence/index/spaces. See [above](#linux_usb_cdc_acm_driver) regarding host USB CDC-ACM drivers.

Implemented in the [usb_cdc_acm_echo.cxx](examples/blue_pill/usb_cdc_acm_echo.cxx) and  [usb_echo.cxx](examples/blue_pill/usb_echo.cxx)
client application example source files.

##### Simple echo
STM32F103 USB device echoes back text sent to it via the a simple custom USB class (see [USB insanity](#usb_standard), below), implemented in [usb_dev_simple.cxx](usb/usb_dev_simple.cxx) Text is prepended with a four-character hexadecimal sequence number, and a two-character hex index showing when a single DOWN (USB host-centric nomenclature) transfer is split into two UP echoes due to the additional 8 bytes of sequence/index/spaces.

Implemented in [usb_simple_echo.cxx](examples/blue_pill/usb_simple_echo.cxx), [usb_echo.cxx](examples/blue_pill/usb_echo.cxx) client application source files. See [linux/stdin.cxx](examples/linux/stdin.cxx) for the simple USB class' `libusb`-based host side user-space "driver".

##### Simple random test
This is a "stress test" demonstration in which both the STM32F103 USB device and the host simultaneously send random data of random length (up to the max 64 byte USB full-speed endpoint data packet limit) to each other using the "simple" custom USB class protocol ([usb_dev_simple.cxx](usb/usb_dev_simple.cxx)). Implemented in the [usb_simple_randomtest.cxx](examples/blue_pill/usb_simple_randomtest.cxx), [usb_randomtest.cxx](examples/blue_pill/usb_randomtest.cxx) client application source files and the [linux/simple_randomtest.cxx](examples/linux/simple_randomtest.cxx) `libusb`-based host-side source file.

##### CDC-ACM random test
Similar to the "simple" randomtest above, but using the standard USB CDC-ACM class ([usb_dev_cdc_acm.cxx](usb/usb_dev_cdc_acm.cxx)) protocol. Note that CDC-ACM uses USB `BULK` endpoints compared to "simple"'s `INTERRUPT`, so performance --- both throughput and latency --- differ greatly between the two. Implemented in the [usb_cdc_acm_randomtest.cxx](examples/blue_pill/usb_cdc_acm_randomtest.cxx), [usb_randomtest.cxx](examples/blue_pill/usb_randomtest.cxx) client application source files and the [linux/tty_randomtest.cxx](examples/linux/tty_randomtest.cxx) `libusb`-based host-side source file.

##### HID mouse
This USB HID demo jitters the host computer's mouse pointer in very small circles (intentionally small, as otherwise regaining control of the host desktop environment would be difficult). Implemented in the [mouse.cxx](examples/blue_pill/mouse.cxx) client application source file and the [usb_dev_hid_mouse.cxx](usb/usb_dev_hid_mouse.cxx) and [usb_dev_hid.cxx](usb/usb_dev_hid.cxx) `UsbDev` C++ derived class sources. See [USB insanity](#usb_standard), below, regarding USB HID class standard.

##### MIDI
This USB MIDI demo sends a looped C major ascending scale to the host computer. ("By pressing down a special key it plays a little melody.") (Not really; code starts immediately and runs indefinitely.) The demo does not implement "MIDI IN" to the STM32F103 device. Source files: [midi.cxx](examples/blue_pill/midi.cxx) and [usb_dev_midi.cxx](usb/usb_dev_midi.cxx). See [USB insanity](#usb_standard), below, regarding USB MIDI class standard.



<br> <a name="implementation_of_papoon_usb"></a>
Implementation of papoon_usb
----------------------------

<a name="cplusplus"></a>
#### C++

Everyone will hate the papoon_usb code.

C coders will hate it because ... C++. Although note that due to papoon_usb's simplistic use of C++, C wrappers can easily be created for access from pure C code. See
[usb_dev.h](usb/usb_dev.h), 
[usb_dev_c.inl](usb/usb_dev_c.inl), 
[usb_dev_cdc_acm_c.cxx](usb/usb_dev_cdc_acm_c.cxx), 
[usb_echo_c.c](examples/blue_pill/usb_echo_c.c), 
[usb_cdc_acm_echo_c.c](examples/blue_pill/usb_cdc_acm_echo_c.c), 
and the `usb_cdc_acm_echo_c.elf` target in
[examples/blue_pill/Makefile](examples/blue_pill/Makefile).

C++ coders will hate papoon_usb because ...

* "That's not C++!! That's 'object based', not 'object-oriented" programming!"
* "init() methods?? **init()** methods!!! That's not RAII!! Why isn't that code in the constructor??"
* "Global scope objects!!?? Why don't you use the Singleton Pattern??"
* "And where are the rest of the design patterns?"
* "Static polymorphism?? Where are the virtual methods?? Why aren't you using dynamic_cast?"

... and many, many more.

Well, there are reasons for why the code was written the way it was, not that those reasons are likely to sway believers in rigid C++ design philosophies. 

Not "object-oriented", only "object-based? Analysis accepted. This is purposeful use of C++ as what some call "a better C".

The init() methods? Again intentional. MCU pre-`main()` startup code is complex enough without requiring that it call runtime constructors for static objects, thus this code's use of only `constexpr` constructors. More important is the fact that much of the object initialization **can't** take place before the MCU itself is configured (clock sources and speeds, peripheral enabling and resetting, etc), and likewise this application-specific MCU initialization  does not belong in the generic pre-`main()` `init()` (aka `start()`) function. Also see [below](#unique_id) for the reason behind the `UsbDev::serial_number_init()` method (this is where the practicalities of the real world diverge from what's taught in CompSci 201).

Global objects vs the Singleton Pattern? The underlying hardware is inherently and unavoidably made up of singletons in the form of hardware subsystems. There is no need to dynamically construct singleton pointers, and in fact the entire software architecture contains no dynamic memory allocation at all (unthinkable!!). I contend this is a rational design in the face of the limited capabilities (20 KB RAM, 64 to 128 KB non-volatile flash memory, 72 MHz max clock speed) of the STM32F103xx chips.

Virtual methods vs static polymorphism? <a name="static_polymorphism"></a> Again, due to memory and processor constraints, avoiding vtable indirection is very desirable. The technique of defining base class methods in derived class implementation (`.cxx`) files (is this a design pattern?) works perfectly well given that only a single derived class will ever be used in any given application executable binary.

But please feel free to re-architect the code to fit other design philosophies. (Respect the GPL open-source [License](#license), above.) Report back on binary executable code size and runtime performance, and if comparable (and I can understand the code) I'll consider using your version.


<br> <a name="regbits_github"></a>
#### regbits

papoon_usb is written using the [regbits](https://github.com/thanks4opensource/regbits) C++ templates for type-safe bit manipulation, and in particular the [regbits_stm](https://github.com/thanks4opensource/regbits_stm) implementation for STM32F103xb, all necessary source files for which are  explicitly/directly included in this repository for convenience (see [regbits.hxx](regbits/regbits.hxx), [stm32f103xb.hxx](regbits/stm32f103xb.hxx)).

Briefly, regbits allows easy (and safe) direct "bare metal" programming of MCU registers:

    rcc->apb1enr |= Rcc::Apb1enr::TIM2EN;  // set single bit
    rcc->cfgr /= Rcc::Cfgr::HPRE_DIV_64;   // set bit field, analogous to
                                           // RCC->CFGR =   (RCC->CFGR & ~HPRE_MASK)
                                                          | HPRE_DIV_64);

but this will not compile:

    rcc->apb1enr |= Rcc::Apb2enr::ADC1EN;  // note mismatch, APB1 vs APB2

See the regbits [README.md](https://github.com/thanks4opensource/regbits/blob/master/README.md) and regbits_stm [README.md](https://github.com/thanks4opensource/regbits_stm/blob/master/README.md) documentation for (much) more detail.



<br> <a name="motivation_for_papoon_usb"></a>
Motivation for papoon_usb
----------------------------

#### Background

"papoon_usb" was written due to ultimate frustration with the open source STM32F103xx USB libraries found online.

The vast majority of those libraries were either copies of one out of two generations of ST provided ones, or were heavily based on them. There were two that were not, and were encouragingly fairly small and cleanly written. Unfortunately they did not work (I don't know how the authors could claim they did), and my attempts at fixing them failed due to lack of understanding of the USB protocols.

Alternately, the USB components of [libopencm3]( https://github.com/libopencm3/libopencm3), the source code for which is somewhat clean and understandable, also did not successfully run when compiled. At that point I decided to start a completely fresh codebase.

Given prior experience with [STM HAL (and LL) libraries](#ST_HAL_and_LL_libraries) --- by coincidence implementing USB on STM32L476 MCUs --- I wanted to avoid them at all cost. (Note that the HAL/STM32L476 success was pure hacking: I extracted a minimal subset of HAL necessary to compile and link, including some small additions such as a `malloc()` stub and a replacement for HAL's `systick` handler. The code was still huge and incomprehensible.) See [STM HAL (and LL) libraries](#ST_HAL_and_LL_libraries) for more, highly-opinionated, criticisms.

I decided instead to use as a starting point a GitHub repository I later discovered was a copy of the older ST-provided "STM32_USB-FS-Device_Lib" source code distribution. Despite the fact the code was functional (after some small fixes), in retrospect this was a very bad decision. The code is, hands down, the second worst I have ever seen in my long career as a software developer. (The worst was a many thousands line long Commodore Basic program 100% comprised of spaghetti code `GOTO` statements. That the ST code is even in the same league is a significant although dubious accomplishment.)  Again, see [ST software](#st_software) for additional complaints.

Finally, in theory none of this analysis/hacking/porting should ever have been necessary. One would think a company the size of ST could document their hardware well enough that a competent programmer could write code to utilize it (and in fact would be interested in doing so, if for no other reason than to increase sales). Not so --- if interested see [STM hardware and documentation](#st_hardware_and_documentation) below for details. Also note that the STM32F103 USB subsystem documentation is better than many other ST attempts, and that evaluation of whether or not I'm a "competent programmer" is left to the reader.

<a name="not_insane"></a>
#### Funny name

I have long thought that the "cuter" the name of a software component, the lower the quality. That said, given the huge amount of time I invested in writing this library (primarily spent decrypting the ST documentation and example code, reverse-engineering the hardware, and dealing with the byzantine  USB standards themselves) --- efforts that never should have been necessary had a reasonable implementation been available --- I chose the name and am including the "insane" accusations.

Please feel free to make your own decisions regarding both the code quality the accusations' merits.

(1) (footnote from repository description [above](#description)) "Papoon" and "Not Insane" are obscure, inside joke, references. My apologies.



<br> <a name="insanity"></a>
Insanity
----------

The following is highly opinionated editorial opinion, perusal of which is not in the least necessary for using or understanding papoon_usb. Those easily offended by extremely critical remarks directed toward well established and respected hardware and software organizations should likely skip reading it. Regardless, I stand by (and attempt to justify) the statements expressed below.

<a name="usb_standard"></a>
### USB standard

The USB standard(s) is/are poorly conceived and (insanely) overly complex.

Period.

As evidence supporting this opinion, search online for the many websites, books, libraries, and blogs attempting to implement and explain USB. (The oft-cited [USB Made Simple](http://www.usbmadesimple.co.uk/) and [USB in a NutShell](https://www.beyondlogic.org/usbnutshell/usb1.shtml) websites are excellent attempts, but even combined are not fully sufficient.)
Also browse the source code for many USB drivers and libraries (possibly including this one) for obvious examples of their authors not fully understanding the systems they're trying to interface with (i.e. code which has all the hallmarks of "this somehow works, don't touch it").

The official standards documents at [USB-IF](https://www.usb.org/) also largely fail. Overly formal "standarese", with few and far between actual illustrative examples, they most frequently describe data fields by name and function but either omit or bury the actual numeric values required for those fields. (The `GET_DESCRIPTOR` `bRreqest` `wValue` contains the "Descriptor Type and
Descriptor Index". Fine. What are those values and their respective meanings? Often such information is contained in completely separate documents, if at all.) (Note that [USB Made Simple - Part 4](file:///usr/local/doc/software/usb/www.usbmadesimple.co.uk/ums_4.htm) has a succinct table listing the values/meanings.) Note also that the USB-IF document 
[Universal Serial Bus Device Class Definition for MIDI Devices](https://www.usb.org/sites/default/files/midi10.pdf) is better than most of their others in this regard.

Beyond this, and more importantly, the problem isn't as much the complexity of the standard(s) *per se* but the lack of hierarchical layering in their design. Okay: Maybe the planet-sized mass of metadata configuration implemented in the [HID Usage Tables](https://www.usb.org/document-library/hid-usage-tables-112) is of value to someone (e.g. a standard API for "Voice Mail Controls" among much similar minutia). Maybe the multiplexed "virtual MIDI cables" (over and above the multiple channels supported by basic MIDI itself) are a useful capability. I think these (and hundreds of others) could have been implemented more simply, but that's immaterial. (See the size and obfuscation of the absolutely required USB descriptors in [usb_dev_hid_mouse.cxx](usb/usb_dev_hid_mouse.cxx) and [usb_dev_midi.cxx](usb/usb_dev_midi.cxx).)

The problem is that all of this complexity is necessary to implement **all** USB classes and clients ... even in the (most frequent) situations where none of it is needed.

Compare, for example, this repository's [usb_dev_simple.cxx](usb/usb_dev_simple.cxx) custom USB class with the   implementation of the standard USB CDC-ACM class in [usb_dev_cdc_acm.cxx](usb/usb_dev_cdc_acm.cxx). The former should be the "Hello, world" of USB classes --- two endpoints, one IN and one OUT, no configuration.

But such a USB class not in the standards, so everyone (as is widely documented) uses CDC-ACM because host-side drivers for it are included in most/all operating systems. Yes, when USB was originally developed, a USB-to-serial bridge device was probably one of the first hardware/software implementations created. But instead of defining all the necessary metadata and out-of-band communications on top of basic communications, it was baked into the fundamental design of the class. So now, many years later, communications between an embedded MCU and a host computer **must** implement baud rate and total/stop/parity bit control even when none of it is relevant to the data being communicated.

Inconsistencies and strange design decisions abound in USB, above and beyond the lack of layering. The basic design sends device description to the host via a small "Device Descriptor" and a large "Configuration Descriptor" containing multiple sub-descriptors (plus optional standalone "String Descriptors"). This is already two mechanisms devoted to the same task -- the host retrieves descriptors by type and index, but also by parsing the Configuration Descriptor for its components. (Possibly this was done for efficiency.) But the mechanism is then extended by various USB classes such as HID which **do** define new "top-level" descriptor types --- which could have been included in the aggregate Configuration Descriptor

Maybe some of this obfuscation was intentional. USB was designed by Intel at the height of the "Wintel" monopoly over the computer industry. (Hmm. Let's see if this repository stays up given the current ownership of GitHub. ;) ). Implementing the desired "plug-and-play" capability would naturally fall to proprietary drivers integrated into a monolithic operating system. Maybe a design which was difficult for others to implement could have been considered "a good thing". ;)

Finally, possibly the most confusing aspect of the very fragile USB enumeration process is the "Set Address" command. The [Universal Serial Bus
Specification Revision 2.0 April 27, 2000](https://www.usb.org/document-library/usb-20-specification) (linked download on page may have URL certificate problems) states:

*9.2.6.3 Set Address Processing*

*After the reset/resume recovery interval, if a device receives a SetAddress() request, the device must be able to complete processing of the request and be able to successfully complete the Status stage of the request within 50 ms. In the case of the SetAddress() request, the Status stage successfully completes when the device sends the zero-length Status packet or when the device sees the
ACK in response to the Status stage data packet.*

*After successful completion of the Status stage, the device is allowed a SetAddress() recovery interval of 2 ms. At the end of this interval, the device must be able to accept Setup packets addressed to the new address. Also, at the end of the recovery interval, the device must not respond to tokens sent to the
old address (unless, of course, the old and new address is the same).*

Any possible mistakes in ST's documentation are not the responsibility of of the USB-IF organization, but [RM0008 Reference manual STM32F101xx, STM32F102xx, STM32F103xx, STM32F105xx and
STM32F107xx advanced ARM ® -based 32-bit MCUs](https://www.st.com/content/ccc/resource/technical/document/reference_manual/59/b9/ba/7f/11/af/43/d5/CD00171190.pdf/files/CD00171190.pdf/jcr:content/translations/en.CD00171190.pdf) states:

*During USB enumeration process, the host assigns a unique address to this device, which must be written in the ADD[6:0] bits of the USB_DADDR register,
and configures any other necessary endpoint.*

Depending on one's level of clairvoyance, these descriptions may or may not contain sufficient information for implementing USB device enumeration. The fact, glossed over in the USB-IF standard and completely omitted in ST's documentation, is that setting the device's address **must** be delayed until the next transfer on the bus occurs. (Trust me. I spent many fine debugging hours discovering this fact.) It is these types of crucial details that make USB so difficult.

And, in this particular case, none of it is necessary. USB is not a "bus" (pedanticism: "not a *multidrop* bus"). It is a multilevel star topology, tree structured, directed acyclic graph network. Any given USB cable is uniquely connected to exactly one upstream hub (root or secondary) and one downstream device. There is no reason for the hub to communicate anything on the cable other than information intended for that device, so why is an "address" needed? The address **is** part of the higher-level device description scheme and thus is needed "above" the hub, but the hub could strip it out for communication to the device, or the device could simply ignore it. Again, this may be an ST hardware implementation issue, but getting it right was absolutely necessary to get papoon_usb to function on the STM32F103xx MCU.


<br>
So why, given all the vitriol expressed above, do I use USB? Simply because there's no other alternative for getting data in and out of a host modern computer. Ethernet's physical layer electrical requirements make it infrequently supported on low-end MCUs. WiFi and Bluetooth took pages from USB's playbook and require similar kinds of unwanted complexity ("Are you a heart rate monitor? Use this Bluetooth profile." No, I just want to send and receive some data, thank you.) And devices like USB-to-serial converters bring one right back to USB (they just hide the details) while adding their own processing and latency issues.

You can't fight city hall.


<br> <a name="st_hardware_and_documentation"></a>
### ST hardware and documentation

As per the example described above, ST's documentation for the USB peripheral, while actually better than many of their other attempts, leaves a lot to be desired. In general the impression given is that users are not intended to have much need for the documentation nor develop much software at the register level it describes, and instead use ST-provided libraries and tools (see [ST software](#st_software), below, for problems in that area).

The USB peripheral in the STM32F10x series MCUs is somewhat primitive and lacking in features/capabilities. Casual research seems to indicate that it was carried over from the smaller STM8 8-bit MCUs. Supporting evidence for these claims is the fact that ST started using a vastly improved USB peripheral in MCUs following the STM32F10x series.


<a name="toggle-only-bits-hardware"></a>
By far the largest problem is the (mis-)design of the "USB endpoint n register (USB_EPnR), n=[0..7]". These 8 registers contain bits that must be accessed and controlled via 4 different methods:

1. A `SETUP` bit that is read-only
2. `CTR_RX` and `CTR_TX` bits that can be read, and cleared to a value of 0, but not set to a value of 1.
3. Normal read-write bits: `EP_TYPE`, `EP_KIND`, and `EA` bits.
4. "Toggle-only" bits that can only be flipped from 1 to 0 or vice versa by writing a 1 value to them (writing 0 has no effect). These include the `DTOG_RX`, `STAT_RX`, `DTOG_TX`, and `STAT_TX` bits.

Of these, type number 4 is the most problematic. [RM0008](https://www.st.com/content/ccc/resource/technical/document/reference_manual/59/b9/ba/7f/11/af/43/d5/CD00171190.pdf/files/CD00171190.pdf/jcr:content/translations/en.CD00171190.pdf) specifically states:

*Read-modify-write cycles on these registers should be avoided because between the read and the write operations some bits could be set by the hardware and the
next write would modify them before the CPU has the time to detect the change.*

This is a logical inconsistency. Several of the "toggle-only" bits need to be set to specific values at various stages of the USB device enumeration process. Their toggle-only access requirement does nothing to change the basic race condition (where the hardware changes them between software read and write) that the documentation warns against. I got involved in a pedantic argument about this when I asked for clarification on the ST support forum --- the responder stated that flipping controlling the bits was not "read-modify-write". That's literally true, but to get them to a desired state requires reading the register's bits, deciding whether they need to be flipped (XOR'd) or not, and writing something back. Same problem.

As evidence of how poorly understood these bits are, see [below](#toggle-only-bits-software) for how ST's own libraries attempt to handle them. My own speculation is that the hardware designers (if they thought through the issue at all) felt that software would know *a priori* what state the bits were in at any given protocol phase, and could then toggle them (or not) as desired to achieve their desired state (see [below](#write-toggle-only-bits)). But note that this **still** doesn't change the existence of the race condition risk.

As the Beatles sang: ["Very strange."](https://youtu.be/S-rB0pHI9fU?t=35)

<a name="unique_id"></a>
Other problems in addition to the above USB peripheral ones include the fact that the STM32F103xx's "Device electronic signature", "Unique device ID register (96 bits)" (see [RM0008](https://www.st.com/content/ccc/resource/technical/document/reference_manual/59/b9/ba/7f/11/af/43/d5/CD00171190.pdf/files/CD00171190.pdf/jcr:content/translations/en.CD00171190.pdf), section 30.2) reads `0xffffffffffffffffffffffff` instead of its correct value when the MCU is clocked at the 72 (or 48) MHz required for USB peripheral operation. Any corroboration or information regarding this is welcome (it is possible that my testing has been done on faulty and/or counterfeit chips).



<br> <a name="st_software"></a>
### ST software

<a name="ST_HAL_and_LL_libraries"></a>
#### ST HAL (and LL) libraries

I've written it before, both in this README.md, similar ones in other repositories, and in emails and online forums: ST's HAL and LL libraries (and their inclusion in the "CubeMX", etc. IDEs) are bloated, inefficient, convoluted disasters masquerading as official, vendor-vetted software.

ST (and other hardware manufactures) **should** distribute code that falls into one of two categories: Either simple, direct, tutorial-like examples that allow developers to understand and better use the hardware, or code that is so fast, efficient, feature-rich and easy for client code to use that no one would need anything else (or to modify them for their own needs). "papoon_usb" may not be perfect (and likely is not), but I contend that it succeeds better on **both** counts than ST's offerings.

I invite interested readers to trace through a function call graph of the USB CDC-ACM device code contained in the
[STM32CubeF1](https://www.st.com/en/embedded-software/stm32cubef1.html) distribution, starting at `Projects/STM3210C_EVAL/Applications/USB_Device/CDC_Standalone/Src/main.c`. I attempted to do so(not for the first time) for this README.md and once again gave up after documenting 2000+ lines worth of program flow.

In all fairness, the HAL code does do more than the [usb_cdc_acm_echo.cxx](examples/blue_pill/usb_cdc_acm_echo.cxx) demo in this repository. (It does implement a classic bidirectional interface between USB and a serial port, and it does use DMA.) But the hallmark of the code is the levels of indirection it contains. Every function calls through multiple software layers ("Projects", "Middlewares", and "Drivers", the latter both HAL and LL). And not in a linear, hierarchical arrangement --- each level freely calls up and down the layers, including cyclical loops through them

Contrast with papoon_usb, where the client application accesses public member functions of the `UsbDev` base class, which, if necessary, invokes derived class (USB class) functionality. Not to mention that the code (a fraction of the HAL/LL size) is contained in a vastly smaller number of files and directories.

Is the ST code either a good tutorial or an efficient implementation? I contend it's neither.


#### STM32_USB-FS-Device_Lib

As "insane" as the current ST Cube/HAL/LL code is, the older "STM32_USB-FS-Device_Lib" is much worse. Again I invite readers to see for themselves, but basically the code defies attempts at static analysis. It is based on a totally unnecessary state machine, driven by a state variable whose value can only be determined at runtime. And tracing USB code at runtime is nearly impossible --- the timing requirements are so strict that any attempts to follow it with debugging breakpoints cause it to fail. I ended up implementing a complex runtime tracing system which kept a large buffer of encoded debug state information and code locations, and standalone programs to parse and display the resultant dumps in readable form. All of this is in addition to the code's fondness for similarly named preprocessor macros and C functions which call each other for no discernable design reason, variables that store values for deferred use later on (these were the first things I removed --- they turned out to be unneeded), and many other mis-features. But as mentioned above, this codebase was the only working one I had to start with.

<a name="toggle-only-bits-software"></a>
#### Endpoint registers "toggle-only" bits

As a final example, both of the above codebases contain almost identical code to handle the "toggle-only" bits in the STM32F103xx USB peripheral as described [above](#toggle-only-bits-hardware). I speculate that the latter code was copied from the former, in an example of the "it works, don't touch it" mentality.

From STM32_USB-FS-Device_Lib's Libraries/STM32_USB-FS-Device_Driver/inc/usb_regs.h:

    #define _SetEPTxStatus(bEpNum,wState) {\
        register uint16_t _wRegVal;       \
        _wRegVal = _GetENDPOINT(bEpNum) & EPTX_DTOGMASK;\
        /* toggle first bit ? */     \
        if((EPTX_DTOG1 & wState)!= 0)      \
          _wRegVal ^= EPTX_DTOG1;        \
        /* toggle second bit ?  */         \
        if((EPTX_DTOG2 & wState)!= 0)      \
          _wRegVal ^= EPTX_DTOG2;        \
        _SetENDPOINT(bEpNum, (_wRegVal | EP_CTR_RX|EP_CTR_TX));    \
      } /* _SetEPTxStatus */


From Cube F1's Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal_pcd.h:

    #define PCD_SET_EP_TX_STATUS(USBx, bEpNum, wState) { register uint16_t _wRegVal;\
       \
        _wRegVal = PCD_GET_ENDPOINT((USBx), (bEpNum)) & USB_EPTX_DTOGMASK;\
       /* toggle first bit ? */     \
       if((USB_EPTX_DTOG1 & (wState))!= 0U)\
       {                                                                            \
         _wRegVal ^= USB_EPTX_DTOG1;        \
       }                                                                            \
       /* toggle second bit ?  */         \
       if((USB_EPTX_DTOG2 & (wState))!= 0U)      \
       {                                                                            \
         _wRegVal ^= USB_EPTX_DTOG2;        \
       }                                                                            \
       PCD_SET_ENDPOINT((USBx), (bEpNum), (_wRegVal | USB_EP_CTR_RX|USB_EP_CTR_TX));\
      } /* PCD_SET_EP_TX_STATUS */

Contrast both of the above with papoon_usb's [stm32f103xb.hxx](regbits/stm32f103xb.hxx):

    void stat_tx(
    const mskd_t    tx_stat)
    volatile
    {
        // don't modify read/write bits, clear clear-only bits, or
        // toggle other toggle-only bits
        Reg<uint32_t, Epr>    current = *this;

        // clear bits which should not be toggled, cleared, or written
                      // must use mskd_t's with all bits set
        current.clr(  Epr::DTOG_TX_DATA1
                    | Epr::CTR_TX
                    | Epr::SETUP
                    | Epr::STAT_RX_VALID
                    | Epr::DTOG_RX_DATA1
                    | Epr::CTR_RX        );

        // XOR new stat bits
        current.flp(tx_stat);

        // write back to register, toggling stat bits to desired value
        *this = current;
    }

A single read, a single `&=` (note that the `|` operands are C++ constexpr; they collapse to a single value at compile time), a single `^`, and a final write. No branching. (Also: This is a C++ inline method, so is as efficient as a C macro.) This code has been shown to work, but I welcome any contrary evidence that the hardware "prefers" the bits be toggled one at a time.




<br> <a name="further_development"></a>
Further development
-----------

#### papoon_usb
* Implement USB standard GET_CONFIGURATION and GET_INTERFACE.
* Try removing cached `ctr_tx` and `ctr_stp` in `UsbDev::ctr()` and rely on bits in `usb->EPRN[0]` register maintaining correct state.
* <a name="write-toggle-only-bits"></a> Try writing endpoint registers' "toggle-only" bits without first reading them for their current vs. desired state, instead trusting that their value is deterministically known at given points in the enumeration process.
* Further investigate performance of CPU vs DMA copies to/from PMA memory.
* Test USB class with multiple configurations in device descriptor.
* Test USB class with multiple interfaces in configuration descriptor (aka "composite" device).
* Implement endpoint double-buffering.


<a name="regbits_future_work"></a>
#### regbits
* Implement direct, write-only, methods for manipulating "toggle only" bits in EPRN[N] registers (rely on register being in known state at fixed points during enumeration process).
* Investigate why [regbits.hxx](regbits/regbits.hxx) `constexpr` semantics require C++17 compilation (`-std=c++17` option) when using the `gcc-arm-none-eabi-9-2019-q4-major` compiler at [arm Developer](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) vs C++11 (`-std=c++11`) being sufficient for the `gcc-arm-none-eabi-8-2018-q4-major` and earlier releases.

#### libusb and Linux
* Find way to fully reset (cause re-enumeration as per power-up/-cycle)  attached STM32F103xx USB peripheral from software as alternative to shorting USB D+ line to ground as per USB hardware standard.
* Find why `simple_randomtest.elf` can simultaneously perform IN and OUT transfers at max USB 2.0 full speed 1 KHz (endpoint descriptor `bInterval`==1) if IN and OUT endpoints in `usb_dev_simple.cxx` have different endpoint numbers, but IN is slower if numbers same (bidirectional endpoint).
* Change endpoint descriptors in `usb_dev_simple.cxx` from `INTERRUPT` to `BULK` and compare performance, both total bandwidth and max latency.
