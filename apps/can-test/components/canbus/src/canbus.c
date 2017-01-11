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
#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#include <utils/zf_log.h>

/* standard */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <utils/ansi.h>
#include <platsupport/delay.h>

#include <camkes.h>

/* application common */
#include "can_inf.h"
#include "spi_inf.h"


// #define CAN_TEST_REPLY


void txb0_ack_callback(void *arg)
{
    txb0_ack_reg_callback(txb0_ack_callback, NULL);
    printf("%s\n", __func__);
}

void txb1_ack_callback(void *arg)
{
    txb1_ack_reg_callback(txb1_ack_callback, NULL);
    printf("%s\n", __func__);
}

void txb2_ack_callback(void *arg)
{
    txb2_ack_reg_callback(txb2_ack_callback, NULL);
    printf("%s\n", __func__);
}

int run(void)
{
    struct can_frame tx, tx1, rx;
    int error = 0;

    error = txb0_ack_reg_callback(txb0_ack_callback, NULL);
    error = txb1_ack_reg_callback(txb1_ack_callback, NULL);
    error = txb2_ack_reg_callback(txb2_ack_callback, NULL);

    /* Initialize CAN controller. */
    printf("Start CAN Test\n");
    can_setup(125000);

    /* Prepare CAN frame. */
    tx.ident.id = 0x200;
    tx.ident.exide = 0;
    tx.ident.rtr = 0;
    tx.dlc = 8;
    tx.data[0] = 0xBC;
    tx.data[1] = 0x22;
    tx.data[2] = 0x00;
    tx.data[3] = 0x9a;
    tx.data[4] = 0x00;
    tx.data[5] = 0x00;
    tx.data[6] = 0x00;
    tx.data[7] = 0x00;

    printf("Requesting packed status\n");
    for (int i = 0; i < 10; i++) {
        error = can_sendto(0, tx);
        printf("Sent: error(%d), id(%x), data(%x, %x, %x, %x, %x, %x, %x, %x)\n",
               error, tx.ident.id,
               tx.data[0], tx.data[1], tx.data[2], tx.data[3],
               tx.data[4], tx.data[5], tx.data[6], tx.data[7]);

        tx.ident.id++;
        tx.data[0] = 0x00;
        tx.data[1] = 0x00;
        tx.data[2] = 0x00;
        tx.data[3] = 0x00;
        tx.data[4] = 0x00;
        tx.data[5] = 0x00;
        tx.data[6] = 0x00;
        tx.data[7] = 0x00;
    }

    while (1) {
        /* Receive message */
        can_recv(&rx);
        ZF_LOGV("Recv: error(%d), id(%x), data(%x, %x, %x, %x, %x, %x, %x, %x)\n",
               error, rx.ident.id,
               rx.data[0], rx.data[1], rx.data[2], rx.data[3],
               rx.data[4], rx.data[5], rx.data[6], rx.data[7]);
    }

    return 0;
}
