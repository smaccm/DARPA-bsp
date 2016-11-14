/*
 * Copyright 2016, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 * @TAG(D61_BSD)
 */

#pragma once

/*
 * MCP2515 instructions and registers
 *
 */

#include <stdint.h>
#include <can_inf.h>

/* Transmit Buffers */
#define TXBCTRL(n)  ((((n) + 0x3) << 4)) //Control register
#define TXBSIDH(n)  (TXBCTRL(n) + 0x1)   //Standard identifier high
#define TXBSIDL(n)  (TXBCTRL(n) + 0x2)   //Standard identifier low
#define TXBEID8(n)  (TXBCTRL(n) + 0x3)   //Extended identifier high
#define TXBEID0(n)  (TXBCTRL(n) + 0x4)   //Extended identifier low
#define TXBDLC(n)   (TXBCTRL(n) + 0x5)   //Data length code

/* Transmit Buffer Control Bits */
#define TXBCTRL_ABTF     BIT(6) //Message aborted flag
#define TXBCTRL_MLOA     BIT(5) //Message lost arbitration
#define TXBCTRL_TXERR    BIT(4) //Transmission error detected
#define TXBCTRL_TXREQ    BIT(3) //Message transmit request
#define TXBCTRL_TXP_SHF  0      //Transmit buffer priority
#define TXBCTRL_TXP_MASK 0x3    //Transmit buffer priority mask

/* Receive Buffers */
#define RXBCTRL(n)  ((((n) + 0x6) << 4)) //Control register
#define RXBSIDH(n)  (RXBCTRL(n) + 0x1)   //Standard identifier high
#define RXBSIDL(n)  (RXBCTRL(n) + 0x2)   //Standard identifier low
#define RXBEID8(n)  (RXBCTRL(n) + 0x3)   //Extended identifier high
#define RXBEID0(n)  (RXBCTRL(n) + 0x4)   //Extended identifier low
#define RXBDLC(n)   (RXBCTRL(n) + 0x5)   //Data length code

/* Receive Buffer Filters */
#define RXFSIDH(n)  ((n) << 2)        //Filter standard identifier high
#define RXFSIDL(n)  (RXFSIDH(n) + 1)  //Filter standard identifier low
#define RXFEID8(n)  (RXFSIDH(n) + 2)  //Filter extended identifier high
#define RXFEID0(n)  (RXFSIDH(n) + 3)  //Filter extended identifier low

#define RXMSIDH(n)  (((n) << 2) + 0x20) //Mask standard identifier high
#define RXMSIDL(n)  (RXMSIDH(n) + 1)    //Mask standard identifier low
#define RXMEIDH(n)  (RXMSIDH(n) + 2)    //Mask extended identifier high
#define RXMEIDL(n)  (RXMSIDH(n) + 3)    //Mask extended identifier low

/* Receive Buffer Control Bits */
#define RXBCTRL_RXM_MASK    0x60   //Receive buffer operating mode mask
#define RXBCTRL_RXM_SHF     5      //Operating mode bit shift
#define RXBCTRL_RTR         BIT(3) //Received remote transfer request
#define RXBCTRL_BUKT        BIT(2) //Rollover enable
#define RXBCTRL_FILHIT_MASK 0x7    //Filter bit mask
#define RXBCTRL_FILHIT_SHF  0      //Filter bit shift

/* Control Registers */
#define CANSTAT  0xE //CAN controller state
#define CANCTRL  0xF //CAN controller control
#define CANINTE  0x2B //CAN interrupt enable
#define CANINTF  0x2C //CAN interrupt flag

#define CANSTAT_OPMOD_SHF  5 //Operation mode
#define CANSTAT_ICOD_SHF   1 //Interrupt flag code

#define CANCTRL_REQOP_MASK  0xE0   //Request operation mode bit mask
#define CANCTRL_REQOP_SHF   5      //Request operation mode
#define CANCTRL_ABAT        BIT(4) //Abort all pending transmissions
#define CANCTRL_OSM         BIT(3) //One shot mode
#define CANCTRL_CLKEN       BIT(2) //CLKOUT pin enable
#define CANCTRL_CLKPRE_SHF  0      //CLKOUT pin prescaler

#define CANINTE_MERRE  BIT(7) //Message error interrupt enable
#define CANINTE_WAKIE  BIT(6) //Wakeup interrupt enable
#define CANINTE_ERRIE  BIT(5) //Error interrupt enable
#define CANINTE_TX2IE  BIT(4) //Transmit buffer 2 empty interrupt enable
#define CANINTE_TX1IE  BIT(3) //Transmit buffer 1 empty interrupt enable
#define CANINTE_TX0IE  BIT(2) //Transmit buffer 0 empty interrupt enable
#define CANINTE_RX1IE  BIT(1) //Receive buffer 1 full interrupt enable
#define CANINTE_RX0IE  BIT(0) //Receive buffer 0 full interrupt enable

#define CANINTF_MERRF  BIT(7) //Message error interrupt flag
#define CANINTF_WAKIF  BIT(6) //Wakeup interrupt flag
#define CANINTF_ERRIF  BIT(5) //Error interrupt flag
#define CANINTF_TX2IF  BIT(4) //Transmit buffer 2 empty interrupt flag
#define CANINTF_TX1IF  BIT(3) //Transmit buffer 1 empty interrupt flag
#define CANINTF_TX0IF  BIT(2) //Transmit buffer 0 empty interrupt flag
#define CANINTF_RX1IF  BIT(1) //Receive buffer 1 full interrupt flag
#define CANINTF_RX0IF  BIT(0) //Receive buffer 0 full interrupt flag

/* Error detection */
#define TEC   0x1C //Transmit error counter
#define REC   0x1D //Receiver error counter
#define EFLG  0x2D //Error flag

#define EFLG_RX1OVR  BIT(7) //Receive buffer 1 overflow flag
#define EFLG_RX0OVR  BIT(6) //Receive buffer 0 overflow flag
#define EFLG_TXBO    BIT(5) //Bus-off error flag
#define EFLG_TXEP    BIT(4) //Transmit error-passive flag
#define EFLG_RXEP    BIT(3) //Receive error-passive flag
#define EFLG_TXWAR   BIT(2) //Transmit error warning flag
#define EFLG_RXWAR   BIT(1) //Receive error warning flag
#define EFLG_EWARN   BIT(0) //Error warning flag

/* Bit timing registers */
#define CNF1  0x2A //Bit timing configuration 1
#define CNF2  0x29 //Bit timing configuration 2
#define CNF3  0x28 //Bit timing configuration 3

#define CNF1_SJW_SHF  6 //Synchronization jump width length
#define CNF1_BRP_SHF  0 //Baud rate prescaler

#define CNF2_BTLMODE     BIT(7) //PS2 bit time length
#define CNF2_SAM         BIT(6) //Sample point configuration
#define CNF2_PHSEG1_SHF  3      //PS1 length
#define CNF2_PRSEG_SHF   0      //Propagation segment length

#define CNF3_SOF         BIT(7) //Start of frame signal
#define CNF3_WAKFIL      BIT(6) //Wake up filter
#define CNF3_PHSEG2_SHF  0      //PS2 length

/* SPI Interface Instruction set */
#define CMD_RESET         0xC0 //Resets internal registers to default state
#define CMD_READ          0x03 //Read data from register
#define CMD_READ_RXB      0x90 //Read RX buffer
#define CMD_WRITE         0x02 //Write data to register
#define CMD_LOAD_TXB      0x40 //Load TX buffer
#define CMD_RTS           0x80 //Request to send
#define CMD_READ_STATUS   0xA0 //Read status
#define CMD_RX_STATUS     0xB0 //RX status
#define CMD_BIT_MODIFY    0x05 //Set or clear individual bits in a particular register

/**
 * Receive Buffer Operating Mode
 * Note: Do *NOT* change the values of these modes.
 */
typedef enum rxm_mode {
    RXM_BOTH = 0,  //Receive all messages, filters enabled
    RXM_SID_ONLY,  //Receive only messages with standard ID
    RXM_EID_ONLY,  //Receive only messages with extended ID
    RXM_ANY        //Receive any messages, turn off masks/filters
} rxm_mode_t;

/**
 * Modes of Operation
 * Note: Do *NOT* change the order of those modes.
 */
typedef enum op_mode {
    REQOP_NORMAL = 0,
    REQOP_SLEEP,
    REQOP_LOOP,
    REQOP_LISTEN,
    REQOP_CONFIG
} op_mode_t;

/* SPI Command Functions */
void mcp2515_reset(void);
uint8_t mcp2515_read_reg(uint8_t reg);
void mcp2515_read_nregs(uint8_t reg, int count, uint8_t *buf);
void mcp2515_write_reg(uint8_t reg, uint8_t val);
void mcp2515_write_nregs(uint8_t reg, uint8_t *buf, int count);
void mcp2515_bit_modify(uint8_t reg, uint8_t mask, uint8_t val);
uint8_t mcp2515_read_status(void);
uint8_t mcp2515_rx_status(void);
void mcp2515_rts(uint8_t mask);
void mcp2515_load_txb(uint8_t *buf, uint8_t len, uint8_t idx, uint8_t flag);
void mcp2515_read_rxb(uint8_t *buf, uint8_t len, uint8_t idx, uint8_t flag);
int poll_rxbuf();

/* MCP2515 functions */
void set_baudrate(uint32_t baudrate);
void set_mode(op_mode_t mode);
op_mode_t get_mode(void);
void load_txb(int txb_idx, struct can_frame *frame);
void recv_rxb(int rxb_idx, struct can_frame *frame);
int set_rx_filter(struct can_id can_id, uint32_t mask);
void clear_rx_filter(uint8_t idx);
void clear_filter_mask(uint8_t rxb_idx);
void abort_tx(int txb_idx);
int txb_status(int txb_idx);
void enable_rollover(void);
void set_rx_mode(int rxb_idx, rxm_mode_t mode);

/* Interrupt functions */
void enable_intrrupt(void);
void start_xmit(void);

/* Caching functions */
void canid_clear_cache(int txb_idx);
int get_frame_priority(int txb_idx, struct can_id *id);
