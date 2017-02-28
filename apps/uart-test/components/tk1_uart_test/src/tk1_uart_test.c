/*
 * Copyright 2016, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(D61_BSD)
 */
#include <string.h>
#include <camkes.h>
#include <sel4utils/sel4_zf_logif.h>
#include <platsupport/plat/serial.h>

#define N_ITERATIONS (256)
#define TEST_XOR_MAGIC  (0x20)

#define SERVER_DEFAULT_BEHAVIOUR 0
#define SERVER_ENABLE_RESTART (true)

#define SERVER_RESTART_CHAR '~'
#define SERVER_NEW_BEHAVIOUR_CHAR0 '!'
#define SERVER_NEW_BEHAVIOUR_CHAR1 '@'
#define SERVER_NEW_BEHAVIOUR_CHAR2 '#'

static void server_echo_once(bool do_xor, bool do_poem);
static void server_echo_incrementing(int n_iterations, bool do_xor);

static char rxdata[N_ITERATIONS * 2];

static int
read_fixed_length(char *inbuff, int uart_id, size_t buffsize, size_t n_chars_to_read)
{
    ssize_t n_chars_read = 0;

    for (; n_chars_read < n_chars_to_read; ) {
        ssize_t status = 0;

        status = tk1_uart_read(uart_id, buffsize);
        if (status < 0) {
            ZF_LOGE("Unexpected read() return value %d.", status);
            continue;
        }

        memcpy(&inbuff[n_chars_read], (void *)tk1_uartb_rx_buff, status);
        n_chars_read += status;
    }

    return n_chars_read;
}

int run(void)
{
    int ret = 0, server_behaviour = SERVER_DEFAULT_BEHAVIOUR;

    printf("UART-tests: Here in main.\n");

    while (true) {
        switch (server_behaviour) {
        case 0:
            server_echo_incrementing(N_ITERATIONS, true);
            break;
        case 1:
            /* Takes anything as input, echoes a poem in response. */
            server_echo_once(false, true);
            break;
        case 2:
            /* Takes anything as input, echoes back the input as response. */
            for (int i = 0; i < N_ITERATIONS; i++) {
                server_echo_once(false, false);
            }
            break;
        default:
            ZF_LOGE("Unknown test %d.", server_behaviour);
            ret = -1;
        }

        ZF_LOGE_IF(SERVER_ENABLE_RESTART,
                   "UART-tests: run: about to enter command loop.");

        while (SERVER_ENABLE_RESTART) {
            ssize_t status;
            bool break_out_and_restart = false;

            status = tk1_uart_read(NV_UARTB_ASYNC, N_ITERATIONS * 2);
            if (status < 0) {
                ZF_LOGE("UART-tests: run: restart loop: read returned %d.",
                        status);
                continue;
            }

            for (int i = 0; i < status; i++) {
                char curr_char;

                curr_char = ((char *)tk1_uartb_rx_buff)[i];
                switch (curr_char) {
                case SERVER_RESTART_CHAR:
                    ZF_LOGE("UART-tests: run: got restart command!");
                    break_out_and_restart = true;
                    break;
                case SERVER_NEW_BEHAVIOUR_CHAR0:
                    ZF_LOGE("UART-tests: run: selected behaviour 0!");
                    server_behaviour = 0;
                    break;
                case SERVER_NEW_BEHAVIOUR_CHAR1:
                    ZF_LOGE("UART-tests: run: selected behaviour 1!");
                    server_behaviour = 1;
                    break;
                case SERVER_NEW_BEHAVIOUR_CHAR2:
                    ZF_LOGE("UART-tests: run: selected behaviour 2!");
                    server_behaviour = 2;
                    break;
                default:
                    ZF_LOGE("UART-tests: Unknown command %x.", curr_char);
                }

                if (break_out_and_restart) {
                    break;
                }
            }

            if (break_out_and_restart) {
                break;
            }
        }
    }

    return ret;
}

/* Listens for incoming data on UARTB, modifies and returns it in response. */
static void
server_echo_incrementing(int n_iterations, bool do_xor)
{
    ssize_t status, total_buffered_bytes=0;

    printf("UART-tests: running incrementing echo test.\n"
            "\t%d iterations. XOR enabled? %d.\n",
            n_iterations, do_xor);

    memset(rxdata, 0, sizeof(rxdata));

    for (int i = 1; i < n_iterations + 1; i++) {
        status = read_fixed_length(&rxdata[total_buffered_bytes], NV_UARTB_ASYNC,
                                   N_ITERATIONS * 2, i);

        if (status < i) {
            ZF_LOGE("UART-tests: read_fixed returned %d.", status);
            continue;
        }

        rxdata[total_buffered_bytes + status] = '\0';
        printf("UART-tests: Read %d bytes from serial on this iteration. "
                "%d bytes left from previous iteration, for total of %d buffered.\n",
                status, total_buffered_bytes, total_buffered_bytes + status);
        printf("Bytes: %s.\n", &rxdata[total_buffered_bytes]);

        /* XOR test: XOR the bytes and send them back out. If you send a capital
         * ascii letter, it should come back as common case, and vice versa.
         */
        if (do_xor) {
            for (int j = total_buffered_bytes;
                 j < total_buffered_bytes + status; j++) {
                rxdata[j] ^= TEST_XOR_MAGIC;
            }
        }

        /* Commit count of total bytes in buffer. */
        total_buffered_bytes += status;

        /* Copy data into tk1_uartb_tx_buff */
        memcpy((void *)tk1_uartb_tx_buff, rxdata, i);

        /* Short delay */
        for (volatile int i = 0; i < 0x100000; i++);

        status = tk1_uart_write(NV_UARTB_ASYNC, i);
        if (status < 0) {
            ZF_LOGW("UART-tests: Unexpected status from WRITE: stat %d.", status);
            continue;
        }

        memmove(rxdata, &rxdata[status], status);
        total_buffered_bytes -= status;

        printf("UART-tests: Wrote %d bytes.\n", status);
    }
}

static const char *fleas_poem = "Big fleas have little fleas\n"
    "Upon their back to bite 'em;\n"
    "And little fleas have smaller fleas\n"
    "And so on ad infinitum.\n";

/* Listens for any data regardless of the amount, and prints what it got to
 * the kernel log.
 *
 * Then it will either echo back to the sender:
 * (1) the same data it received, or
 * (2) a short poem about fleas.
 */
static void
server_echo_once(bool do_xor, bool do_poem)
{
    ssize_t status, n_bytes_to_write;

    printf("UART-tests: running mirror/poem echo test.\n"
            "\tXOR enabled? %d. Poem? %d.\n",
            do_xor, do_poem);

    status = tk1_uart_read(NV_UARTB_ASYNC, N_ITERATIONS * 2);
    if (status < 0) {
        ZF_LOGE("UART-tests: read returned %d.", status);
        return;
    }

    /* memcpy to the rxdata static buffer so we can NULL terminate for the
     * sake of printing.
     */
    memcpy(rxdata, (void *)tk1_uartb_rx_buff, status);
    rxdata[status] = '\0';
    printf("UART-tests: echo test: received '%s'.\n", rxdata);

    if (!do_poem) {
        n_bytes_to_write = status;
        memcpy((void *)tk1_uartb_tx_buff, (void *)tk1_uartb_rx_buff, status);
    } else {
        n_bytes_to_write = strlen(fleas_poem);
        memcpy((void *)tk1_uartb_tx_buff, fleas_poem, n_bytes_to_write);
    }

    if (do_xor) {
        for (int i = 0; i < n_bytes_to_write; i++) {
            ((char *)tk1_uartb_tx_buff)[i] ^= TEST_XOR_MAGIC;
        }
    }

    status = tk1_uart_write(NV_UARTB_ASYNC, n_bytes_to_write);
    if (status < 0) {
        ZF_LOGE("UART-tests: write returned %d.", status);
        return;
    }

    printf("UART-tests: Wrote %d bytes of %d.\n", status, n_bytes_to_write);
}
