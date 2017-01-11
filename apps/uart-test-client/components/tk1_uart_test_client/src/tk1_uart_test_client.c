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

#define CONFIG_TEST_READ_RESPONSE       (true)
#define CONFIG_TEST_CHECK_RESPONSE_XOR  (false)

static char inbuff[N_ITERATIONS * 2];

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
    int ret=0;
    memset(inbuff, 0, sizeof(inbuff));

    ZF_LOGE("UART-client: about to begin tests.");

    for (int i = 1; i <= N_ITERATIONS; i++) {
        int status;

        for (int j = 0; j < i; j++) {
            ((char *)tk1_uartb_tx_buff)[j] = 'U';
        }

        status = tk1_uart_write(TK1_UARTB_ASYNC, i);
        if (status < i) {
            ZF_LOGE("UART-client: write returned %d when trying to write %d bytes.",
                    status, i);

            continue;
        }

        ZF_LOGE("UART-client: Wrote %d bytes.", status);

        if (CONFIG_TEST_READ_RESPONSE) {
            status = read_fixed_length(inbuff, TK1_UARTB_ASYNC, N_ITERATIONS * 2, i);
            if (status < 0) {
                ZF_LOGE("UART-client: read_fixed returned %d when trying to "
                        "read %d bytes.",
                        status, i);
                ret = -1;
                continue;
            }

            inbuff[status] = '\0';
            ZF_LOGE("UART-client: response of %dB: %s.", status, inbuff);

            if (CONFIG_TEST_CHECK_RESPONSE_XOR) {
                for (int j = 0; j < i; j++) {
                    if (inbuff[j] < 'a' || inbuff[j] > 'z') {
                        ZF_LOGE("Got non-lowercase ascii char %x.", inbuff[j]);
                        ret = -1;
                    }
                }
            }
        }
        else {
            /* If we're not waiting for a response from the other end, we should
             * introduce a delay between each write, or else we will send too
             * fast in our tight loop here, and swamp the other end.
             */
            for (volatile int i = 0; i < 0x1000000; i++);
        }
    }

    return ret;
}
