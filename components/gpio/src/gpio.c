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


mux_sys_t tegra_mux;
gpio_t i_spi_can_int;
gpio_t r_spi_can1_cs;
gpio_t r_spi_can2_cs;
gpio_sys_t gpio_sys;

void irq_grp5_int_handle(void)
{
    if (gpio_is_pending(&i_spi_can_int)) {
        CAN1Int_emit();
    } else {
        gpio_pending_clear(&i_spi_can_int);
        irq_grp5_int_acknowledge();
    }
}

void CANIntAck_handler(void *arg UNUSED)
{
    CANIntAck_reg_callback(CANIntAck_handler, NULL);
    gpio_pending_clear(&i_spi_can_int);
    irq_grp5_int_acknowledge();
}

void gpio_spi_can1_cs(const int enable)
{
    if (enable) {
        ZF_LOGD("Enabling chip select");
        gpio_set(&r_spi_can1_cs);
    } else {
        ZF_LOGD("Disabling chip select");
        gpio_clr(&r_spi_can1_cs);
    }
}

void gpio__init(void)
{
    CANIntAck_reg_callback(CANIntAck_handler, NULL);

    ZF_LOGD("Initialising mux");
    tegra_mux_init(pinmuxmisc, pinmuxaux, &tegra_mux );
    ZF_LOGD("Mux initialised\nInitialising gpio");
    gpio_init(gpio1base, &tegra_mux, &gpio_sys);
    gpio_new(&gpio_sys, CAN1_INTn, GPIO_DIR_IRQ_FALL, &i_spi_can_int);
    gpio_new(&gpio_sys, CAN1_CS, GPIO_DIR_OUT, &r_spi_can1_cs);
    gpio_new(&gpio_sys, CAN2_CS, GPIO_DIR_OUT, &r_spi_can2_cs);
    ZF_LOGD("Gpio initialised");
}
