/*
 * Copyright 2016, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 * @TAG(D61_BSD)
 */

/* standard */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <utils/ansi.h>
#include <platsupport/delay.h>

#include "canbus.h"

/* application common */
#include "can_inf.h"
#include "spi_inf.h"


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
    printf( A_FG_B "Start CAN Test\n" A_FG_RESET );
    can_setup(125000);

    /* Prepare CAN frame. */
    tx.ident.id = 0x123;
    tx.ident.exide = 0;
    tx.ident.rtr = 0;
    tx.dlc = 8;
#if 0
    tx.data[0] = 0x08;
    tx.data[1] = 0x07;
    tx.data[2] = 0x06;
    tx.data[3] = 0x05;
    tx.data[4] = 0x04;
    tx.data[5] = 0x03;
    tx.data[6] = 0x02;
    tx.data[7] = 0x01;
#endif
    tx.data[0] = 0x01;
    tx.data[1] = 0x02;
    tx.data[2] = 0x03;
    tx.data[3] = 0xDE;
    tx.data[4] = 0xAD;
    tx.data[5] = 0xBE;
    tx.data[6] = 0xEF;
    tx.data[7] = 0x08;

    tx1.ident.id = 0x321;
    tx1.ident.exide = 0;
    tx1.ident.rtr = 0;
    tx1.dlc = 8;

    tx1.data[0] = 0xA1;
    tx1.data[1] = 0xB2;
    tx1.data[2] = 0xC3;
    tx1.data[3] = 0xDE;
    tx1.data[4] = 0xEA;
    tx1.data[5] = 0xFE;
    tx1.data[6] = 0x1F;
    tx1.data[7] = 0x28;

    int sendcount = 0;

    error = 0;

    printf( A_FG_B "starting canbus loop.....\n" A_FG_RESET );
    while (1) {
        /* Send message */
        tx.ident.id = 0x300;

        if (sendcount < 10) {
            printf( A_FG_B  "Sending Can data: %d\n" A_FG_RESET, __LINE__ );
            error = can_sendto(0, tx);
            sendcount++;
        }
        if (error) {
            printf( A_FG_B  "Start CAN line: %d\n" A_FG_RESET, __LINE__ );
            can_abort(0);
            printf( A_FG_B "Send error: 0\n" A_FG_RESET );
            printf( A_FG_B  "Start CAN line: %d\n" A_FG_RESET, __LINE__ );
        }
        /* Receive message */
        can_recv(&rx);
        printf("Recv: error(%d), id(%x), data(%x, %x, %x, %x, %x, %x, %x, %x)\n",
               error, rx.ident.id,
               rx.data[0], rx.data[1], rx.data[2], rx.data[3],
               rx.data[4], rx.data[5], rx.data[6], rx.data[7]);

        ps_udelay(999999);
        printf( A_FG_B  "sleep done line: %d\n" A_FG_RESET, __LINE__ );

        if (sendcount < 10) {
            error = can_sendto(0, tx1);
            sendcount++;
        }
    }

    return 0;
}
