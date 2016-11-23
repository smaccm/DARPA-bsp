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
#include <platsupport/plat/serial.h>

#define N_ITERATIONS (128)
#define TEST_XOR_MAGIC  (0x20)

const char *msg = "Big fleas have little fleas\n"
                  "Upon their backs to bite 'em.\n"
                  "And little fleas have smaller fleas;\n"
                  "And so on, ad infinitum.\n";

/* Listens for incoming data on UARTB. Then writes a poem out in response. */
int run(void)
{
    int status;

    ZF_LOGE("UART-tests: Here in main.\n");

    uintptr_t buffpos = 0;
    for (int i = 1; i < N_ITERATIONS + 1; i++) {
        while (buffpos < i) {
            /* Listen for data */
            status = tk1_uart_read(TK1_UARTB, 2048);
            if (status < 0) {
                ZF_LOGW("UART-tests: Unexpected status from read: stat %d.", status);
                continue;
            }

            memcpy((uint8_t *)tk1_uartb_tx_buff + buffpos, (uint8_t *)tk1_uartb_rx_buff, status);
            buffpos += status;
        }

        ((char *)tk1_uartb_tx_buff)[buffpos] = '\0';
        ZF_LOGE("UART-tests: Read %d bytes TOTAL from serial. Bytes: %s.", buffpos, (char *)tk1_uartb_tx_buff);

        /* XOR test: XOR the bytes and send them back out. If you send a capital
         * ascii letter, it should come back as common case, and vice versa.
         */
        for (int j = 0; j < buffpos; j++) {
            ((uint8_t *)tk1_uartb_tx_buff)[j] ^= TEST_XOR_MAGIC;
        }

        status = tk1_uart_write(TK1_UARTB, i);
        if (status < 0) {
            ZF_LOGW("UART-tests: Unexpected status from WRITE: stat %d.", status);
            continue;
        }

        memmove((void *)tk1_uartb_tx_buff, (uint8_t *)tk1_uartb_tx_buff + status, buffpos - status);
        buffpos -= status;
        ZF_LOGE("UART-tests: Wrote %d bytes.", status);
    }

    return 0;
}
