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

#include <platsupport/mux.h>
#include <platsupport/gpio.h>
#include <can_inf.h>
#include <utils/ansi.h>
#include <utils/zf_log.h>
#include <camkes.h>


#define gpio_dev_debug(fmt, args...) \
    do { \
           ZF_LOGD( A_BG_C "  %s: " A_BG_RESET, __func__); \
           ZF_LOGD( fmt, ##args); \
           ZF_LOGD("\n"); \
    } while(0)

mux_sys_t tegra_mux;
gpio_t i_spi_can_int;
gpio_t r_spi_can1_cs;
gpio_t r_spi_can2_cs;
gpio_sys_t gpio_sys;

static void
irq_grp5_event(void* arg)
{
    if (gpio_is_pending(&i_spi_can_int)) {
        gpio_pending_clear(&i_spi_can_int);
        CAN1Int_emit();
    }
    irq_grp5_int_reg_callback(&irq_grp5_event, NULL);
}

void gpio_spi_can1_cs(const int enable)
{
    if (enable) {
        gpio_dev_debug("line: %d\n", __LINE__);
        gpio_set(&r_spi_can1_cs);
    } else {
        gpio_dev_debug("line: %d\n", __LINE__);
        gpio_clr(&r_spi_can1_cs);
    }
}

void gpio__init(void)
{
    gpio_dev_debug("line: %d\n", __LINE__);
    tegra_mux_init(pinmuxmisc, pinmuxaux, &tegra_mux );
    gpio_dev_debug("line: %d\n", __LINE__);
    gpio_init(gpio1base, &tegra_mux, &gpio_sys);
    gpio_new(&gpio_sys, CAN1_INTn, GPIO_DIR_IRQ_FALL, &i_spi_can_int);
    gpio_new(&gpio_sys, CAN1_CS, GPIO_DIR_OUT, &r_spi_can1_cs);
    gpio_new(&gpio_sys, CAN2_CS, GPIO_DIR_OUT, &r_spi_can2_cs);
    gpio_dev_debug("line: %d\n", __LINE__);
    irq_grp5_int_reg_callback(&irq_grp5_event, NULL);
    gpio_dev_debug("line: %d\n", __LINE__);
}
