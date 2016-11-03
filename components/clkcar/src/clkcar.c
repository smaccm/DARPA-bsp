/*
 * Copyright 2016, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 * @TAG(D61_BSD)
 */

#include <stdio.h>
#include <stdint.h>
#include <clkcar.h>
#include <utils/zf_log.h>

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
    clk_source_spi_reg |= 190;
    *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_CLK_SOURCE_SPI1_0_OFFSET) = clk_source_spi_reg;

    /*Enable SPI controller */
    clk_reset_reg = *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_RST_DEVICES_H_0_OFFSET);
    clk_reset_reg &= ~(1 << SWR_SPI1_RST_SHIFT);
    *(volatile uint32_t*)(local_clkcar_reg + CLK_RST_CONTROLLER_RST_DEVICES_H_0_OFFSET) = clk_reset_reg;
}

void
clkcar_uart_clk_init(void)
{

}
