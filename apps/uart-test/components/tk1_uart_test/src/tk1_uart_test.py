#! /usr/bin/env python

#
# Copyright 2016, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
#
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
#
# @TAG(D61_BSD)
#

import serial
import time
import sys
import os

if len(sys.argv) != 2:
    sys.exit('Usage:\n\ttk1_uart_test.py <TTY_FILE_NAME>')

if not os.path.exists(sys.argv[1]):
    sys.exit('TTY file specified does not exist.')

ser = serial.Serial(sys.argv[1], 115200)
for j in range(1000):
    for i in range(1, 256 + 1):
        text = ('ABCDEFGHIJKLMNOPQRSTUVWXYZ' * 10)[:i]
        ser.write(text)
        print j, i, "Sent:", text
        resp = ser.read(i)
        print j, i, "Response:", resp
        assert text.lower() == resp
