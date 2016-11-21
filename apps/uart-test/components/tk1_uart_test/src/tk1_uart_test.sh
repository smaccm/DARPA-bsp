#! /usr/bin/env bash

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

# Test for the uart driver.
#
# Runs in a loop 256 times, and each time sends an incrementally greater number
# of characters over the UART. It expects to get those characters back with
# XORd with 0x20. XORing ASCII characters with 0x20 flips their case, so
# XORing upper-case 'U' with 0x20 will yield lower-case 'u', and so on.
#
# To use the test, run the CAmkES component as part of your CAmkES application,
# and then when the CAmkES component has booted, connect a cable to the
# TK1 that is running the CAmkES component, and then run this script using the
# command-line:
#  tk1_uart_test.sh <TTY_DEVICE_FILE_NAME>
#
# Where TTY_DEVICE_FILE_NAME is the /dev/tty* file that represents your
# connection to the UART on which the test component is listening.
#

if [ "$#" != "1" ]
then
    echo "Usage: $0 <TTY_DEVICE_FILENAME>"
    exit 1;
fi

echoedbytesFile=`mktemp`
loop_iterations=256
declare -a testbytes;
readerPid=''

function atexit {
    rm -f $echoedbytesFile
}

trap atexit EXIT

for i in `seq $loop_iterations`
do
    # Generate array.
    testbytes=`printf 'U%.0s' \`seq $i\``

    # Run the read in the bg so it gets all characters asynchronously
    timeout 2 head -c $i <$1 >$echoedbytesFile &
    readerPid=$!

    echo "Testing $i bytes."
    echo -n "$testbytes" >"$1"
    wait $readerPid
    echo "Returned bytes: " `cat $echoedbytesFile`
done

exit 0
