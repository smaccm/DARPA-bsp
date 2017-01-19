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

#include <stdio.h>
#include <platsupport/delay.h>
#include <camkes.h>

void play(int freqHz, int durationMs) {
    if (freqHz == 0) {
        ps_mdelay(durationMs);
        return;
    }

    int halfPeriodUs = 500000 / freqHz;
    int count = (1000 * durationMs) / (2 * halfPeriodUs);
    for (int i = 0; i < count; i++) {
        gpio_spi_can1_cs(0);
        ps_udelay(halfPeriodUs);
        gpio_spi_can1_cs(1);
        ps_udelay(halfPeriodUs);
    }
}

int run(void) {
    // Close enough
    ps_cpufreq_hint(680 * 1000 * 1000);

    int baseDuration = 1500;

    while (1) {
        play(440, baseDuration / 4);
        play(659, baseDuration / 32);
        play(440, baseDuration / 8);
        play(1046, baseDuration / 4);
        play(0, baseDuration / 8);
        play(440, baseDuration / 4);
        play(0, baseDuration / 8);
        play(1046, baseDuration / 4);
        play(440, baseDuration / 32);
        play(1046, baseDuration / 8);
        play(1318, baseDuration / 4);
        play(0, baseDuration / 8);
        play(1046, baseDuration / 4);
        play(0, baseDuration / 8);
        play(1318, baseDuration / 4);
        play(1046, baseDuration / 32);
        play(1318, baseDuration / 8);
        play(1568, baseDuration / 4);
        play(0, baseDuration / 8);
        play(784, baseDuration / 4);
        play(0, baseDuration / 8);
        play(1046, baseDuration / 4);
        play(784, baseDuration / 32);
        play(1046, baseDuration / 8);
        play(1318, baseDuration / 2);
        play(0, baseDuration / 4);

        ps_sdelay(2);
    }
}
