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
#include <sel4utils/sel4_zf_logif.h>

/* Struct that describes the init status of, and holds relevant metadata about,
 * each of the UARTs. Includes pointers to the shmem dataports, etc.
 * 
 * We only support the asynchronous versions (E-G).
 */
struct cdev_desc {
    struct ps_chardevice    inst, *pinst;
    volatile void           *tx_shmem_buff, *rx_shmem_buff;
    int (*state_mtx_lock)(void);
    int (*state_mtx_unlock)(void);
    int (*write_sync_mtx_lock)(void);
    int (*write_sync_mtx_unlock)(void);
    int (*read_sync_mtx_lock)(void);
    int (*read_sync_mtx_unlock)(void);

    bool do_release_write_sync_mutex;
    bool do_release_read_sync_mutex;
} static cdevs[3];

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
    cdevs[0].rx_shmem_buff = tk1_uarta_rx_buff;
    cdevs[0].state_mtx_lock = &tk1_uarta_state_mtx_lock;
    cdevs[0].state_mtx_unlock = &tk1_uarta_state_mtx_unlock;
    cdevs[0].write_sync_mtx_lock = &tk1_uarta_write_sync_mtx_lock;
    cdevs[0].write_sync_mtx_unlock = &tk1_uarta_write_sync_mtx_unlock;
    cdevs[0].read_sync_mtx_lock = &tk1_uarta_read_sync_mtx_lock;
    cdevs[0].read_sync_mtx_unlock = &tk1_uarta_read_sync_mtx_unlock;
    cdevs[0].do_release_write_sync_mutex = false;
    cdevs[0].do_release_read_sync_mutex = false;

    cdevs[1].tx_shmem_buff = tk1_uartb_tx_buff;
    cdevs[1].rx_shmem_buff = tk1_uartb_rx_buff;
    cdevs[1].state_mtx_lock = &tk1_uartb_state_mtx_lock;
    cdevs[1].state_mtx_unlock = &tk1_uartb_state_mtx_unlock;
    cdevs[1].write_sync_mtx_lock = &tk1_uartb_write_sync_mtx_lock;
    cdevs[1].write_sync_mtx_unlock = &tk1_uartb_write_sync_mtx_unlock;
    cdevs[1].read_sync_mtx_lock = &tk1_uartb_read_sync_mtx_lock;
    cdevs[1].read_sync_mtx_unlock = &tk1_uartb_read_sync_mtx_unlock;
    cdevs[1].do_release_write_sync_mutex = false;
    cdevs[1].do_release_read_sync_mutex = false;

    cdevs[2].tx_shmem_buff = tk1_uartc_tx_buff;
    cdevs[2].rx_shmem_buff = tk1_uartc_rx_buff;
    cdevs[2].state_mtx_lock = &tk1_uartc_state_mtx_lock;
    cdevs[2].state_mtx_unlock = &tk1_uartc_state_mtx_unlock;
    cdevs[2].write_sync_mtx_lock = &tk1_uartc_write_sync_mtx_lock;
    cdevs[2].write_sync_mtx_unlock = &tk1_uartc_write_sync_mtx_unlock;
    cdevs[2].read_sync_mtx_lock = &tk1_uartc_read_sync_mtx_lock;
    cdevs[2].read_sync_mtx_unlock = &tk1_uartc_read_sync_mtx_unlock;
    cdevs[2].do_release_write_sync_mutex = false;
    cdevs[2].do_release_read_sync_mutex = false;

    error = camkes_io_ops(&tk1_uart_io_ops);
    if (error != 0) {
        ZF_LOGF("TK1-UART: Failed to get IO-ops.");
    }

    error = tk1_uarta_write_sync_mtx_lock();
    assert(error == 0);
    error = tk1_uartb_write_sync_mtx_lock();
    assert(error == 0);
    error = tk1_uartc_write_sync_mtx_lock();
    assert(error == 0);

    error = tk1_uarta_read_sync_mtx_lock();
    assert(error == 0);
    error = tk1_uartb_read_sync_mtx_lock();
    assert(error == 0);
    error = tk1_uartc_read_sync_mtx_lock();
    assert(error == 0);
}

static void
tk1_uart_irq_handler(int uart_id)
{
    struct cdev_desc *mydev;

    assert(uart_id >= TK1_UARTA_ASYNC && uart_id <= TK1_UARTC_ASYNC);
    uart_id -= TK1_UARTA_ASYNC;

    mydev = &cdevs[uart_id];
    assert(mydev->pinst != NULL);

    mydev->state_mtx_lock();

    ps_cdev_handle_irq(&mydev->inst, 0);
    mydev->state_mtx_unlock();
    if (mydev->do_release_write_sync_mutex == true) {
        /* This `if` block is about making sure we deal with the locks in the
         * right order. The most natural and intuitive design would be for us
         * to release the write_sync_mtx lock inside the callback function we
         * supply to the platsupport-driver.
         *
         * However, this could mean that we would wake up the other thread
         * before we release the state_mtx.
         *
         * We need to release the state_mtx before we release the write_sync_mtx.
         * So basically instead of unlocking the write_sync_mtx in our callback,
         * we just set a bool-flag in our callback, and float the responsibility
         * for the lock ordering back up to the CAmkES IRQ handler (this
         * function right here.).
         *
         * TL;DR: much ado about lock ordering.
         */
        mydev->do_release_write_sync_mutex = false;
        mydev->write_sync_mtx_unlock();
    }
    if (mydev->do_release_read_sync_mutex == true) {
        /* Same as the write-side operation: lock ordering. */
        mydev->do_release_read_sync_mutex = false;
        mydev->read_sync_mtx_unlock();
    }
}

void
tk1_uarta_irq_handle(void)
{
    int error;

    tk1_uart_irq_handler(TK1_UARTA_ASYNC);
    error = tk1_uarta_irq_acknowledge();
    ZF_LOGE_IF(error != 0, "Failed to acknowledge UART-A IRQ!");
}

void
tk1_uartb_irq_handle(void)
{
    int error;

    tk1_uart_irq_handler(TK1_UARTB_ASYNC);
    error = tk1_uartb_irq_acknowledge();
    ZF_LOGE_IF(error != 0, "Failed to acknowledge UART-B IRQ!");
}

void
tk1_uartc_irq_handle(void)
{
    int error;

    tk1_uart_irq_handler(TK1_UARTC_ASYNC);
    error = tk1_uartc_irq_acknowledge();
    ZF_LOGE_IF(error != 0, "Failed to acknowledge UART-C IRQ!");
}

static void
tk1_uart_write_cb(ps_chardevice_t* d, enum chardev_status stat,
                  size_t bytes_transfered, void* token)
{
    int uart_id = d->id;

    assert(uart_id >= TK1_UARTA_ASYNC && uart_id < TK1_UARTD_ASYNC);
    uart_id -= TK1_UARTA_ASYNC;

    if (d->write_descriptor.bytes_requested > 0) {
        cdevs[uart_id].do_release_write_sync_mutex = true;
    }
}

int
tk1_uart_write(int uart_num, int nbytes)
{
    int ret;

    /* Don't allow access to UART-D either. */
    if (uart_num < TK1_UARTA_ASYNC || uart_num >= TK1_UARTD_ASYNC) {
        return -1;
    }
    uart_num -= TK1_UARTA_ASYNC;

    if (cdevs[uart_num].tx_shmem_buff == NULL) {
        ZF_LOGE("Dataport not connected!");
        return -1;
    }

    /* This mutex ensures that two simultaneous calls to write or to read and
     * write, won't access the regs at the same time.
     */
    cdevs[uart_num].state_mtx_lock();

    if (cdevs[uart_num].pinst == NULL) {

        clkcar_uart_clk_init(uart_num + TK1_UARTA_ASYNC);
        cdevs[uart_num].pinst = ps_cdev_init(uart_num + TK1_UARTA_ASYNC,
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
                        &tk1_uart_write_cb, NULL);

    cdevs[uart_num].state_mtx_unlock();

    if (ret < 0) {
        return ret;
    }

    /* This mutex serializes the asynchronous driver API write call. */
    cdevs[uart_num].write_sync_mtx_lock();

    return cdevs[uart_num].inst.write_descriptor.bytes_transfered;
}

static void
tk1_uart_read_cb(ps_chardevice_t* d, enum chardev_status stat,
                  size_t bytes_transfered, void* token)
{
    int uart_id = d->id;

    assert(uart_id >= TK1_UARTA_ASYNC && uart_id < TK1_UARTD_ASYNC);
    uart_id -= TK1_UARTA_ASYNC;

    if (d->read_descriptor.bytes_requested > 0) {
        cdevs[uart_id].do_release_read_sync_mutex = true;
    }
}

int
tk1_uart_read(int uart_num, int nbytes)
{
    int ret;

    /* Don't allow access to UART-D either. */
    if (uart_num < TK1_UARTA_ASYNC || uart_num >= TK1_UARTD_ASYNC) {
        return -1;
    }
    uart_num -= TK1_UARTA_ASYNC;

    if (cdevs[uart_num].rx_shmem_buff == NULL) {
        ZF_LOGE("Dataport not connected!");
        return -1;
    }

    /* This mutex ensures that two simultaneous calls to read, or to read and
     * write, won't access the regs at the same time.
     */
    cdevs[uart_num].state_mtx_lock();


    if (cdevs[uart_num].pinst == NULL) {
        clkcar_uart_clk_init(uart_num + TK1_UARTA_ASYNC);
        cdevs[uart_num].pinst = ps_cdev_init(uart_num + TK1_UARTA_ASYNC,
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
                       &tk1_uart_read_cb, NULL);

    cdevs[uart_num].state_mtx_unlock();

    if (ret < 0) {
        return ret;
    }

    cdevs[uart_num].read_sync_mtx_lock();

    return cdevs[uart_num].inst.read_descriptor.bytes_transfered;
}
