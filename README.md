<!--
#
# Copyright 2016, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
# @TAG(D61_BSD)
#
-->

This repository contains device driver components and tests for the TK1-SOM 
and daughterboard used in phase 3 of the DARPA HACMS SMACCM project for the 
quadcopter air vehicle.

The componetns are in `components/` and include:
- CAN
- SPI
- GPIO
- UART

There are apps in `apps/` to test these, including:
- can-test
- uart-test
