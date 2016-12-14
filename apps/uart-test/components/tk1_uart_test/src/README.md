# Tests for the UART components

There are two components involved here: there's a test server and a test client.
You choose the test that you'd like to run by configuring the server component
and then you run your tests using either the client component, or some other
client that can give the server the input it's expecting.

The idea is that you connect two TK1 boards via a UART cable, and have two
separate images running on each one; one should run the server and the other
should run the client.

The tests are all currently set up to use UART-B by default. Feel free the
change this in the sources if you'd like.

## Server component:

Can be found in `apps/uart-test/components/tk1_uart_test/src`

To build this component, run `make uart-test_defconfig && make`

The server component can be configured to respond to several different tests.
To change the behaviour of the server, you'll need to change it in the
source code and rebuild it.

The three tests that are available are:

 * "Incremental echo test": The server will sit and listen for activity on
UART-B. First it listens for 1 byte, then XORs it with a value and echoes it
back out over UART-B. Then it'll listen for 2 bytes, XOR them, and echo them
back out over UART-B. Then do the same for 3 bytes, 4 bytes, and so on. The
Client component can be used as the client for this test, or you can use the
python script (`tk1_uart_test.py`) in the Client component's `src` directory
to act as the client on a Linux machine.

 * "Poem echo test": The server will sit and listen for activity on UART-B.
Upon receiving one byte regardless of the value of that byte, it will just
write a 4-line poem back out over the line, and then exit. The idea here is that
you run this in `minicom` from a Linux machine as the client, and you run the
CAmkES server component on a TK1 that is connected to that Linux machine.

 * "Echo test": The server will sit and listen for activity on UART-B. Upon
receiving any data, it will echo back that data verbatim over the line. The idea
here is also that you run this in `minicom` on a Linux machine acting as the
client, and run the server component on a TK1 that's connected to the Linux
machine.

## Client component:

Can be found in `apps/uart-test-client/components/tk1_uart_test_client/src`

To build this component, run `make uart-test-client_defconfig && make`

The client component only corresponds to one of the tests in the server
component, specifically the "incremental echo test".

The other tests are meant to be executed in a console emulator, such as
perhaps `minicom`.

# Selecting the test you want to configure the test-server component to run:

This section pertains to how to set up the server component to run the test you
want it to run.

You should open up `apps/uart-test/components/tk1_uart_test/src/tk1_uart_test.c`
and look at the `#define`s at the top.

## For the "Incremental echo" test:
 * Set `SERVER_DEFAULT_BEHAVIOUR` to `0` to select the the "Incremental echo"
test.
 * Change `N_ITERATIONS` to change how many incremental steps the test server
 will listen for.

## For the "Poem echo" test:
 * Set `SERVER_DEFAULT_BEHAVIOUR` to `1` to select the the "Poem echo"
test.

## For the "Echo test":
 * Set `SERVER_DEFAULT_BEHAVIOUR` to `2` to select the the "Incremental echo"
test.
 * Change `N_ITERATIONS` to change how many times the server will spin listening
 for input (and therefore how many times it will echo that input back to you).

```
Tip: Don't forget to recompile!
```

# Running the "Incremental echo" test on two TK1s

This section really just deals with the fact that when you power-cycle a board,
some garbage bytes get sent out on the UART line, so when running the tests you
need to take that into consideration. A typical way to factor this into your
execution of the "Incremental Echo" test between two TK1s might be:

 * Power on the "client" TK1, and when u-boot says to "Press any key", press a
 key so that the board doesn't boot into the client CAmkES/seL4 image.
 * Power on the "server" TK1, and allow it to boot into its CAmkES/seL4 image.
 * Now return to the client TK1, and type the relevant u-boot commands to cause
 it to finally boot into the CAmkES/seL4 image and begin the test.
