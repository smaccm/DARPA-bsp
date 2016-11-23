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
#include <assert.h>
#include <string.h>
#include <camkes.h>
#include <camkes/io.h>
#include <platsupport/plat/serial.h>
#include <platsupport/chardev.h>

/* Struct that describes the init status of, and holds relevant metadata about,
 * each of the UARTs. Includes pointers to the shmem dataports, etc.
 */
struct cdev_desc {
    struct ps_chardevice    inst, *pinst;
    volatile void           *tx_shmem_buff, *rx_shmem_buff;
} static cdevs[4];

static ps_io_ops_t tk1_uart_io_ops;

void
tk1_uart__init(void)
{
    int error;

    /* We deliberately don't expose UARTD because that's the one the kernel is
     * using.
     */
    ZF_LOGV("TK1 UART interface initialized. Regs at %p\n", tk1_uart_regs);

    memset(cdevs, 0, sizeof(cdevs));

    cdevs[0].tx_shmem_buff = tk1_uarta_tx_buff;
    cdevs[1].tx_shmem_buff = tk1_uartb_tx_buff;
    cdevs[2].tx_shmem_buff = tk1_uartc_tx_buff;

    cdevs[0].rx_shmem_buff = tk1_uarta_rx_buff;
    cdevs[1].rx_shmem_buff = tk1_uartb_rx_buff;
    cdevs[2].rx_shmem_buff = tk1_uartc_rx_buff;

    error = camkes_io_ops(&tk1_uart_io_ops);
    if (error != 0) {
        ZF_LOGF("TK1-UART: Failed to get IO-ops.");
    }
}

int
tk1_uart_write(int uart_num, int nbytes)
{
    int ret;

    /* Don't allow access to UART-D either. */
    if (uart_num < TK1_UARTA || uart_num >= TK1_UARTD) {
        return -1;
    }

    if (cdevs[uart_num].tx_shmem_buff == NULL) {
        ZF_LOGE("Dataport not connected!");
        return -1;
    }

    /* This mutex ensures that two simultaneous calls to write or to read and
     * write, won't access the regs at the same time.
     */
    tk1_uart_state_mtx_lock();

    if (cdevs[uart_num].pinst == NULL) {

        clkcar_uart_clk_init(uart_num);
        cdevs[uart_num].pinst = ps_cdev_init(uart_num,
                                             /* tk1_uart_regs is the MMIO
                                              * mapping dataport.
                                              */
                                             &tk1_uart_io_ops,
                                             &cdevs[uart_num].inst);
        if (cdevs[uart_num].pinst == NULL) {
            return -1;
        }
    }

    ret = ps_cdev_write(cdevs[uart_num].pinst,
                        (void *)cdevs[uart_num].tx_shmem_buff, nbytes,
                        NULL, NULL);

    tk1_uart_state_mtx_unlock();
    return ret;
}

int
tk1_uart_read(int uart_num, int nbytes)
{
    int ret;

    /* Don't allow access to UART-D either. */
    if (uart_num < TK1_UARTA || uart_num >= TK1_UARTD) {
        return -1;
    }

    if (cdevs[uart_num].rx_shmem_buff == NULL) {
        ZF_LOGE("Dataport not connected!");
        return -1;
    }

    /* This mutex ensures that two simultaneous calls to read, or to read and
     * write, won't access the regs at the same time.
     */
    tk1_uart_state_mtx_lock();

    if (cdevs[uart_num].pinst == NULL) {
        clkcar_uart_clk_init(uart_num);
        cdevs[uart_num].pinst = ps_cdev_init(uart_num,
                                             /* tk1_uart_regs is the MMIO
                                              * mapping dataport.
                                              */
                                             &tk1_uart_io_ops,
                                             &cdevs[uart_num].inst);
        if (cdevs[uart_num].pinst == NULL) {
            return -1;
        }
    }

    ret = ps_cdev_read(cdevs[uart_num].pinst,
                       (void *)cdevs[uart_num].rx_shmem_buff, nbytes,
                       NULL, NULL);

    tk1_uart_state_mtx_unlock();
    return ret;
}
