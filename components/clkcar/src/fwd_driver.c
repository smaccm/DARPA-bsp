/*
* Copyright 2017, Data61
* Commonwealth Scientific and Industrial Research Organisation (CSIRO)
* ABN 41 687 119 230.
*
* This software may be distributed and modified according to the terms of
* the BSD 2-Clause license. Note that NO WARRANTY is provided.
* See "LICENSE_BSD2.txt" for details.
*
* @TAG(D61_BSD)
*/

// #define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#include <stdio.h>

#include <camkes.h>

#include <platsupport/plat/clock_indexes.h>
#include <platsupport/clock.h>

#define PLLCON(a, NAME) (a >= NAME##_START && a < NAME##_END)

typedef enum access_type {
    READ_ACCESS,
    WRITE_ACCESS
} access_type_t;

typedef struct access_request {
    size_t addr;
    access_type_t access_type;
    uint32_t value;
} access_request_t;

typedef bool (*allow_access_fn)(access_request_t *access_request, const clk_register_t *clk_register);


#define PLLP_BASE_0 0xa0
#define PLLP_MISC_0 0xac

/* Handler for pll registers.  We let linux control all plls, apart from PLLP which we block
    PLLP_BASE and PLLP_MISC which contains enable, lock and voltage control bits */
bool allow_pll(access_request_t *access_request, const clk_register_t *clk_register) {
    // PLLC
    uint32_t addr = access_request->addr;
    if (PLLCON(addr, PLLC)) {
        ZF_LOGV("pllc");
    } else if (PLLCON(addr, PLLM)) {
        ZF_LOGV("pllm");
    } else if (PLLCON(addr, PLLP)|| PLLCON(addr, PLLP_1) || addr == PLLP_RESHIFT_0) {
        ZF_LOGV("pllp");
        if ((addr == PLLP_BASE_0 || addr == PLLP_MISC_0) && access_request->access_type == WRITE_ACCESS) {
            ZF_LOGV("Pllp write 0x%x", access_request->value);
            return false;
        }
    } else if (PLLCON(addr, PLLA)) {
        ZF_LOGV("plla");
    } else if (PLLCON(addr, PLLU)) {
        ZF_LOGV("pllu");
    } else if (PLLCON(addr, PLLD)) {
        ZF_LOGV("plld");
    } else if (PLLCON(addr, PLLX)|| PLLCON(addr, PLLX_1)|| PLLCON(addr, PLLX_2)) {
        ZF_LOGV("pllx");
    } else if (PLLCON(addr, PLLE) || addr == PLLE_AUX_0) {
        ZF_LOGV("plle");
    } else if (PLLCON(addr, PLLD2)|| PLLCON(addr, PLLD2_1)) {
        ZF_LOGV("plld2");
    } else if (PLLCON(addr, PLLREFE)) {
        ZF_LOGV("pllrefe");
    } else if (PLLCON(addr, PLLC2)) {
        ZF_LOGV("pllc2");
    } else if (PLLCON(addr, PLLC3)) {
        ZF_LOGV("pllc3");
    } else if (PLLCON(addr, PLLDP)) {
        ZF_LOGV("plldp");
    } else if (PLLCON(addr, PLLC4)) {
        ZF_LOGV("pllc4");
    } else {
        ZF_LOGE("Unknown PLL address");
    }
    return true;
}

/* Handler for enable and reset registers.  Allows read and write access to all but
    blocks write access if SPI1 or UARTB devices are affected
    TODO There are likely a whole bunch of other devices that need to be blocked */
bool allow_enbrst(access_request_t *access_request, const clk_register_t *register_type) {
    register_bank_t rb = register_type->eb.rb;
    uint32_t value = access_request->value;

    ZF_LOGV("enable/reset register");
    if (access_request->access_type == WRITE_ACCESS) {

        uint32_t base_id = rb * 32;
        for (int i = 0; value != 0 && i < 32; i++) {
            int pos = __builtin_ctz(value);
            uint32_t device = base_id + pos;
            /* Only update register if all bits are allowed registers */
            switch (device) {
            case CLK_ENB_UARTD:
            case CLK_ENB_SDMMC4:
            case CLK_ENB_USB2:
            case CLK_ENB_USB3:
                break;
            default:
                ZF_LOGV("Blocking device: %d", device);
                return false;
                break;
            }
            value &= ~BIT(pos);
            ZF_LOGV("|%d %d %x|", rb, pos, value);
        }
    }
    return true;
}

/* Handler for CLK_RST_CONTROLLER_CLK_SOURCE_* registers.  Allows read access to all
    but blocks write access to uartb and spi1 source registers */
bool allow_source(access_request_t *access_request, const clk_register_t *register_type) {
    ZF_LOGV("source register");
    switch (register_type->st.clks) {
        case uartd_r_clk:
        case sdmmc4_r_clk_t:
            return true;
        default:
            if (access_request->access_type == WRITE_ACCESS) {
                ZF_LOGV("Tried to configure clk %d", register_type->st.clks);
                return false;
            } else {
                return true;
            }
    }
    return false;
}

/* Handler for miscellanious registers.  Allows read access but not write access */
bool allow_misc(access_request_t *access_request, const clk_register_t *register_type) {
    ZF_LOGV("misc register");
    if (access_request->access_type == READ_ACCESS) {
        return true;
    } else {
        return false;
    }
}

/* Handler for reserved registers.  Returns false */
bool allow_reserved(access_request_t *access_request, const clk_register_t *register_type) {
    ZF_LOGE("Unknown reserved register");

    return false;
}

allow_access_fn access_handlers[] = {
    [CLK_RESERVED] = allow_reserved,
    [CLK_PLL] = allow_pll,
    [CLK_ENBRST_DEVICES] = allow_enbrst,
    [CLK_SOURCE] = allow_source,
    [CLK_MISC] = allow_misc,
};
compile_time_assert(handlers_for_all_register_types,
    (ARRAY_SIZE(access_handlers)) == NUM_REGISTER_TYPES);

/**
 * Handler function for deciding whether a hardware register can be read or written
 *
 * The register type is looked up from tk1_clk_registers (defined in libplatsupport)
 * Based on the register type an access_handler will be called.
 * The handler will return a boolean indicating whether the read or write can
 * be performed.
 *
 * @param addr        offset into frame
 * @param value       value to read or write.  If access_type is READ_ACCESS
 *                     then *value is 0.
 * @param access_type READ_ACCESS or WRITE_ACCESS
 */
void handle_register(size_t addr, uint32_t *value, access_type_t access_type) {
    if (value == NULL) {
        ZF_LOGF("NULL pointer");
    }
    ZF_LOGV("Offset: 0x%x, ", addr);

    if ((addr % sizeof(uintptr_t)) != 0) {
        ZF_LOGE("Bad offset 0x%x\n", addr);
        return;
    }

    uintptr_t index = addr / sizeof(uintptr_t);
    clk_register_type_t regtype = tk1_clk_registers[index].reg_type;

    if (regtype >= NUM_REGISTER_TYPES || regtype <= CLK_RESERVED) {
        ZF_LOGW("Invalid register type");
        return;
    }

    access_request_t ar = {.addr = addr, .value = *value, .access_type = access_type};
    allow_access_fn handler = access_handlers[regtype];
    if (handler == NULL) {
        ZF_LOGF("Null handler");
    }
    bool allow_action = handler(&ar, &tk1_clk_registers[index]);


    if (allow_action) {
        uint8_t *base_addr = (uint8_t *)clkcar_reg;
        uint32_t *update = (uint32_t *)(base_addr + addr);

        if (access_type == READ_ACCESS) {
            *value = *update;
        } else if (access_type == WRITE_ACCESS) {
            *update = *value;
        } else {
            ZF_LOGF("Invalid access type");
        }
    }
}

/**
 * Return value at offset `addr` in the CAR device frame.
 *
 * @param  addr offset into frame.  Must be < PAGE_SIZE
 * @return      Value at hardware address
 */
uint32_t gen_fwd_read(size_t addr) {
    if (addr >= PAGE_SIZE) {
        ZF_LOGE("Invalid address given: 0x%08x\n", addr);
        return 0;
    }

    uint32_t valueupdate = 0;
    handle_register(addr, &valueupdate, READ_ACCESS);

    return valueupdate;
}

/**
 * Writes `value` to offset `addr` in the CAR device frame.
 *
 * @param  addr offset into frame.  Must be < PAGE_SIZE
 * @param  value value to write.
 */
void gen_fwd_write(size_t addr, uint32_t value)
{
    if (addr >= PAGE_SIZE) {
        ZF_LOGE("Invalid address given");
        return;
    }

    uint32_t valueupdate = value;
    handle_register(addr, &valueupdate, WRITE_ACCESS);
}

void gen_fwd__init() {
    ZF_LOGI("Init clkcar camkes acess control interface");
}
