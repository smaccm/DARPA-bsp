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
#include <stdint.h>
#include <camkes.h>
#include <utils/zf_log.h>
#include <utils/arith.h>
#include <platsupport/plat/serial.h>

#define CLK_RESET_PADDR 0x60006000
#define CLK_RST_CONTROLLER_RST_DEVICES_H_0_OFFSET 0x8
#define SWR_SPI1_RST_SHIFT 9
#define SWR_SPI1_RST (1 << SWR_SPI1_RST_SHIFT)

#define CLK_RST_CONTROLLER_CLK_OUT_ENB_H_0 0x14
#define CLK_ENB_SPI1_SHIFT 9
#define CLK_ENB_SPI1 (1 << CLK_ENB_SPI1_SHIFT)

#define CLK_RST_CONTROLLER_CLK_SOURCE_SPI1_0_OFFSET 0x134
#define CLK_RST_CONTROLLER_CLK_OUT_ENB_H_0_OFFSET 0x14
#define CLK_RST_CONTROLLER_CLK_SOURCE_SHIFT 29

#define CLK_RST_CONTROLLER_CLK_SOURCE_SPI1_0_OFFSET 0x134
#define CLK_RST_CONTROLLER_CLK_OUT_ENB_H_0_OFFSET 0x14
#define CLK_RST_CONTROLLER_CLK_SOURCE_SHIFT 29

#define TK1_RESET_DEVICES_L0_UARTA_SHIFT        (6)
#define TK1_RESET_DEVICES_L0_UARTB_SHIFT        (7)
#define TK1_RESET_DEVICES_H0_UARTC_SHIFT        (23)
#define TK1_RESET_DEVICES_U0_UARTD_SHIFT        (1)

#define TK1_CLKOUT_ENABLE_L0_UARTA_SHIFT        (6)
#define TK1_CLKOUT_ENABLE_L0_UARTB_SHIFT        (7)
#define TK1_CLKOUT_ENABLE_H0_UARTC_SHIFT        (23)
#define TK1_CLKOUT_ENABLE_U0_UARTD_SHIFT        (1)

static volatile void *local_clkcar_reg;

void
clkcar__init(void)
{
    local_clkcar_reg = clkcar_reg;
}


/* We assume that the clock frequency is set by uboot and is unchanged during operation.
    We also assume that all devices operate at the same bus frequency.*/
void clkcar_spi_clk_init(void)
{

    /* Disable SPI controller */
    uint32_t clk_reset_reg = *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_RST_DEVICES_H_0_OFFSET);
    clk_reset_reg |= (1 << SWR_SPI1_RST_SHIFT);
    *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_RST_DEVICES_H_0_OFFSET) = clk_reset_reg;

    /* Enable SPI clock */
    uint32_t clk_enb_reg = *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_CLK_OUT_ENB_H_0);
    clk_enb_reg |= CLK_ENB_SPI1;
    *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_CLK_OUT_ENB_H_0) = clk_enb_reg;

    /* Set input clock and divisor */
    uint32_t clk_source_spi_reg = *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_CLK_SOURCE_SPI1_0_OFFSET);
    clk_source_spi_reg |= (6 << CLK_RST_CONTROLLER_CLK_SOURCE_SHIFT);
    clk_source_spi_reg |= 1;
    *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_CLK_SOURCE_SPI1_0_OFFSET) = clk_source_spi_reg;

    /*Enable SPI controller */
    clk_reset_reg = *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_RST_DEVICES_H_0_OFFSET);
    clk_reset_reg &= ~(1 << SWR_SPI1_RST_SHIFT);
    *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_RST_DEVICES_H_0_OFFSET) = clk_reset_reg;
}

static uint32_t
tk1_clk_source_uart_encode(uint8_t clksource, uint16_t divisor, bool enable_divisor)
{
    return ((clksource & 0x7) << 29)
           | ((enable_divisor & 0x1) << 24)
           | (divisor & 0xFF);
}

/** @file Clock and Reset controller access routines.
 *
 * Provides routines to map the CAR controller, and enable devices and set
 * clock frequencies. Needs a ps_io_ops_t (for ps_io_map()/ps_io_unmap()).
 */
static void
clock_uart_init(const char *uart_id_char,
                volatile uint32_t *car_reset_reg, uint8_t car_reset_reg_shift,
                volatile uint32_t *car_clkout_reg, uint8_t car_clkout_reg_shift,
                volatile uint32_t *car_clksrc_switch_reg)
{
    /* Disable the device */
    uint32_t car_reset_regval;
    ZF_LOGV("Disabling UART%s.\n", uart_id_char);
    car_reset_regval = *car_reset_reg;
    car_reset_regval |= BIT(car_reset_reg_shift);
    *car_reset_reg = car_reset_regval;

    /* Disable the clock source */
    uint32_t car_clkout_regval;
    ZF_LOGV("Disabling UART%s clock source.\n", uart_id_char);
    car_clkout_regval = *car_clkout_reg;
    car_clkout_regval &= ~BIT(car_clkout_reg_shift);
    *car_clkout_reg = car_clkout_regval;

    /* Program the clock source by setting the mux for the device */
    uint32_t car_clksrc_switch_regval;
    ZF_LOGV("Programming UART%s clock switch/mux.\n", uart_id_char);
    car_clksrc_switch_regval = tk1_clk_source_uart_encode(0, 2, false);
    *car_clksrc_switch_reg = car_clksrc_switch_regval;

    /* Re-enable the clock source */
    ZF_LOGV("Re-enabling UART%s clock source.\n", uart_id_char);
    car_clkout_regval = *car_clkout_reg;
    car_clkout_regval |= BIT(car_clkout_reg_shift);
    *car_clkout_reg = car_clkout_regval;

    /* Re-enable the device */
    ZF_LOGV("Re-enabling UART%s.\n", uart_id_char);
    car_reset_regval = *car_reset_reg;
    car_reset_regval &= ~BIT(car_reset_reg_shift);
    *car_reset_reg = car_reset_regval;
}

typedef struct tk1_car_regs {
    volatile uint32_t    car_rst_clk_sources;   /* 0 */
    volatile uint32_t    car_rst_devices_l0;    /* 4 */
    volatile uint32_t    car_rst_devices_h0;    /* 8 */
    volatile uint32_t    car_rst_devices_u0;    /* 12 */
    volatile uint32_t    car_clkout_enable_l0;  /* 16 */
    volatile uint32_t    car_clkout_enable_h0;  /* 20 */
    volatile uint32_t    car_clkout_enable_u0;  /* 24 */
    PAD_STRUCT_BETWEEN(0x18, 0x178, uint32_t);
    volatile uint32_t    car_clk_source_uarta;  /* 0x178 */
    volatile uint32_t    car_clk_source_uartb;  /* 0x17C */
    PAD_STRUCT_BETWEEN(0x17c, 0x1a0, uint32_t);
    volatile uint32_t    car_clk_source_uartc;  /* 0x1A0 */
    PAD_STRUCT_BETWEEN(0x1a0, 0x1c0, uint32_t);
    volatile uint32_t    car_clk_source_uartd;  /* 0x1C0 */
} tk1_car_regs_t;

tk1_car_regs_t *
get_car_regs(volatile void *dataport)
{
    return (tk1_car_regs_t *)dataport;
}

static void
clock_uarta_init(void)
{
    tk1_car_regs_t *car_regs = get_car_regs(clkcar_reg);

    clock_uart_init("A",
                    &car_regs->car_rst_devices_l0,
                    TK1_RESET_DEVICES_L0_UARTA_SHIFT,
                    &car_regs->car_clkout_enable_l0,
                    TK1_CLKOUT_ENABLE_L0_UARTA_SHIFT,
                    &car_regs->car_clk_source_uarta);
}

static void
clock_uartb_init(void)
{
    tk1_car_regs_t *car_regs = get_car_regs(clkcar_reg);

    clock_uart_init("B",
                    &car_regs->car_rst_devices_l0,
                    TK1_RESET_DEVICES_L0_UARTB_SHIFT,
                    &car_regs->car_clkout_enable_l0,
                    TK1_CLKOUT_ENABLE_L0_UARTB_SHIFT,
                    &car_regs->car_clk_source_uartb);
}

static void
clock_uartc_init(void)
{
    tk1_car_regs_t *car_regs = get_car_regs(clkcar_reg);

    clock_uart_init("C",
                    &car_regs->car_rst_devices_h0,
                    TK1_RESET_DEVICES_H0_UARTC_SHIFT,
                    &car_regs->car_clkout_enable_h0,
                    TK1_CLKOUT_ENABLE_H0_UARTC_SHIFT,
                    &car_regs->car_clk_source_uartc);
}

int
clkcar_uart_clk_init(int id)
{
    assert(id >= NV_UARTA_ASYNC && id < NV_UARTD_ASYNC);

    /* UART-D isn't used, and because it's held by the kernel, so we don't
     * expose it.
     */
    switch (id) {
    case NV_UARTA_ASYNC:
        clock_uarta_init();
        break;
    case NV_UARTB_ASYNC:
        clock_uartb_init();
        break;
    default: /* NV_UARTC_ASYNC */
        clock_uartc_init();
        break;
    }

    return 0;
}
