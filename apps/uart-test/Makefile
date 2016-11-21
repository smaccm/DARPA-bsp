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

ADL := uart-test.camkes

include clkcar/clkcar.mk 
include tk1_uart/tk1_uart.mk

tk1_uart_test_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/tk1_uart_test/include/*.h))
tk1_uart_test_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/tk1_uart_test/src/*.c))


include ${PWD}/tools/camkes/camkes.mk
