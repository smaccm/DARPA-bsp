#
# Copyright 2016, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
# @TAG(D61_BSD)
#

TARGETS := $(notdir ${SOURCE_DIR}).cdl

ADL := DARPA.camkes

spi_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/spi/include/*.h)) \
		$(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/include/*.h))
spi_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/spi/src/*.c))

tk1_uart_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/tk1_uart/include/*.h))
tk1_uart_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/tk1_uart/src/*.c))

tk1_uart_test_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/tk1_uart_test/include/*.h))
tk1_uart_test_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/tk1_uart_test/src/*.c))

canbus_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/include/*.h)) \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/canbus/include/*.h))
canbus_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/canbus/src/*.c))

can_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/can/include/*.h))\
		$(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/include/*.h))

can_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/can/src/*.c))

gpio_HFILES := \
     $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/gpio/include/*.h)) \
		 $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/include/*.h))

gpio_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/gpio/src/*.c))

clkcar_HFILES := \
     $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/clkcar/include/*.h)) \
		 $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/include/*.h))

clkcar_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/clkcar/src/*.c))


spi_LIBS += platsupport sel4sync
include ${PWD}/tools/camkes/camkes.mk
