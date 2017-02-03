#
# Copyright 2017, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
# @TAG(D61_BSD)
#

TARGETS := $(notdir ${SOURCE_DIR}).cdl

ADL := vm.camkes

include spi/spi.mk
include can/can.mk
include gpio/gpio.mk
include clkcar/clkcar.mk
include tk1_uart/tk1_uart.mk
include canbus/canbus.mk
include tk1_uart_test/tk1_uart_test.mk

TEMPLATES := templates/global-templates templates/darpa-templates
include TimeServerTK1/TimeServer.mk
include TimeClient/TimeClient.mk
# Subdirectory containing extra templates.


include VM/VM.mk
