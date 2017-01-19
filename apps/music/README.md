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

CAmkES components and tests for SMACCM phase 3 on TK1-SOM.
Runs a CAN server that prints out received CAN messages and then
responds with fixed CAN messages.  

To test from a separate Linux instance:
~~~~
ubuntu@tegra-ubuntu:~$ sudo ip link set can0 up type can bitrate 125000
ubuntu@tegra-ubuntu:~$ cansend can0 5A1#11.22.33.44.55.66.77.88
ubuntu@tegra-ubuntu:~$ candump any
  can0  321   [8]  A1 B2 C3 DE EA FE 1F 28
  can0  300   [8]  01 02 03 DE AD BE EF 08
~~~~

Output from CAmkES serial
~~~~
Bootstrapping kernel
Start CAN Test
CAN device started...
mcp2515 spi_transfer done
starting canbus loop.....
Sending Can data: 104
txb0_ack_callback
Recv: error(0), id(5a1), data(11, 22, 33, 44, 55, 66, 77, 88)
sleep done line: 122
Sending Can data: 104
txb0_ack_callback
txb0_ack_callback
~~~~
