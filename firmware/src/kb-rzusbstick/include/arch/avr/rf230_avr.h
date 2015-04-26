// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  AVR specific header file for the RF230 radio transceiver driver.
 *
 * \par Application note:
 *      AVR2017: RZRAVEN FW
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Id: rf230_avr.h 41219 2008-05-01 10:51:43Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef RF230_AVR_H
#define RF230_AVR_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "board.h"
/*================================= MACROS           =========================*/
#define rf230_set_slptr_high()     (PORT_SLP_TR |= (1 << SLP_TR))
#define rf230_set_slptr_low()      (PORT_SLP_TR &= ~(1 << SLP_TR))
#define rf230_get_slptr()          ((PIN_SLP_TR & (1 << SLP_TR)) >> SLP_TR)


#define rf230_set_rst_high()       (PORT_RST |= (1 << RST))
#define rf230_set_rst_low()        (PORT_RST &= ~(1 << RST))
#define rf230_get_rst()            ((PIN_RST & (1 << RST)) >> RST)


#define rf230_set_tst_high()       (PORT_TST |= (1 << TST))
#define rf230_set_tst_low()        (PORT_TST &= ~(1 << TST))
#define rf230_get_tst()            ((PIN_TST & (1 << TST)) >> TST)


#define RF230_SS_HIGH()            RF230_PORT_SPI |= (1 << RF230_DD_SS)
#define RF230_SS_LOW()             RF230_PORT_SPI &= ~(1 << RF230_DD_SS)


#define RF230_DISABLE_SPI_MODULE() (RF230_SPI_PRR |= (1 << RF230_SPI_PRR_BIT))
#define RF230_ENABLE_SPI_MODULE()  (RF230_SPI_PRR &= ~(1 << RF230_SPI_PRR_BIT))


#define RF230_ENABLE_TRX_ISR() RF230_INTERRUPT_ENABLE()
#define RF230_DISABLE_TRX_ISR() (RF230_INTERRUPT_DISABLE())


#define RF230_WAIT_FOR_SPI_TX_COMPLETE()    do {                           \
    while ((RF230_SPI_STATUS_REG & (1 << RF230_SPI_DONE_FLAG)) == 0) { ; } \
} while (0)


#define RF230_QUICK_REGISTER_READ(reg_addr, reg_value) do { \
    RF230_SS_LOW();                                         \
                                                            \
    RF230_SPI_DATA_REG = (RF230_TRX_CMD_RR | reg_addr);     \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                       \
    reg_value = RF230_SPI_DATA_REG;                         \
                                                            \
    RF230_SPI_DATA_REG = reg_value;                         \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                       \
    reg_value = RF230_SPI_DATA_REG;                         \
    RF230_SS_HIGH();                                        \
} while (0)


#define RF230_QUICK_REGISTER_WRITE(reg_addr, reg_value) do { \
    RF230_SPI_DATA_REG = (reg_addr | RF230_TRX_CMD_RW);      \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                        \
    uint8_t dummy = RF230_SPI_DATA_REG;                      \
                                                             \
    RF230_SPI_DATA_REG = reg_value;                          \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                        \
    dummy = RF230_SPI_DATA_REG;                              \
    RF230_SS_HIGH();                                         \
} while (0)


#define RF230_QUICK_SUBREGISTER_READ(addr, mask, pos, reg_value) do { \
    RF230_SS_LOW();                                                   \
                                                                      \
    RF230_SPI_DATA_REG = (RF230_TRX_CMD_RR | addr);                   \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                                 \
    reg_value = RF230_SPI_DATA_REG;                                   \
                                                                      \
    RF230_SPI_DATA_REG = reg_value;                                   \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                                 \
    reg_value = RF230_SPI_DATA_REG;                                   \
    RF230_SS_HIGH();                                                  \
                                                                      \
    reg_value &= mask;                                                \
    reg_value >>= pos;                                                \
} while (0)


#define RF230_QUICK_SUBREGISTER_WRITE(addr, mask, pos, reg_value) do { \
    RF230_SS_LOW();                                                    \
                                                                       \
    RF230_SPI_DATA_REG = (RF230_TRX_CMD_RR | addr);                    \
                                                                       \
    uint8_t new_reg_value = reg_value;                                 \
    new_reg_value <<= pos;                                             \
    new_reg_value &= mask;                                             \
                                                                       \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                                  \
    uint8_t current_register_value = RF230_SPI_DATA_REG;               \
                                                                       \
    RF230_SPI_DATA_REG = current_register_value;                       \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                                  \
    current_register_value = RF230_SPI_DATA_REG;                       \
    RF230_SS_HIGH();                                                   \
                                                                       \
    RF230_SS_LOW();                                                    \
                                                                       \
    RF230_SPI_DATA_REG = (RF230_TRX_CMD_RW | addr);                    \
                                                                       \
    current_register_value &= ~mask;                                   \
    new_reg_value |= current_register_value;                           \
                                                                       \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                                  \
    current_register_value = RF230_SPI_DATA_REG;                       \
                                                                       \
    RF230_SPI_DATA_REG = new_reg_value;                                \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                                  \
    current_register_value = RF230_SPI_DATA_REG;                       \
    RF230_SS_HIGH();                                                   \
} while (0)


#define RF230_QUICK_READ_FRAME_LENGTH(frame_length) do { \
    RF230_SS_LOW();                                      \
                                                         \
    RF230_SPI_DATA_REG = RF230_TRX_CMD_FR;               \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                    \
    frame_length = RF230_SPI_DATA_REG;                   \
                                                         \
    RF230_SPI_DATA_REG = frame_length;                   \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                    \
    frame_length = (RF230_SPI_DATA_REG & 0x7F);          \
    RF230_SS_HIGH();                                     \
} while (0)


#define RF230_QUICK_FRAME_READ(frame_ptr) do {        \
    RF230_SS_LOW();                                   \
                                                      \
    /* Send the Frame Read command */                 \
    RF230_SPI_DATA_REG = RF230_TRX_CMD_FR;            \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                 \
    uint8_t frame_length = RF230_SPI_DATA_REG;        \
                                                      \
    RF230_SPI_DATA_REG = frame_length;                \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                 \
    frame_length = (RF230_SPI_DATA_REG & 0x7F);       \
                                                      \
    uint8_t *frame = frame_ptr;                       \
    frame_length++;                                   \
                                                      \
    RF230_SPI_DATA_REG = 0x00;                        \
                                                      \
    RF230_WAIT_FOR_SPI_TX_COMPLETE();                 \
                                                      \
    /* Upload the frame and LQI. */                   \
    do {                                              \
        uint8_t const temp_data = RF230_SPI_DATA_REG; \
        RF230_SPI_DATA_REG = temp_data;               \
        RF230_WAIT_FOR_SPI_TX_COMPLETE();             \
        *frame++ = temp_data;                         \
        frame_length--;                               \
    } while (0 != frame_length);                      \
    RF230_SS_HIGH();                                  \
} while (0)


#define RF230_QUICK_CLEAR_ISR_CALLBACK() do { \
    rf230_callback_handler = NULL;            \
} while (0)


#define RF230_QUICK_ISR_HANDLER() do {                 \
    RF230_ACK_INTERRUPT_FLAG();                        \
    uint8_t isr_src = 0;                               \
    RF230_QUICK_REGISTER_READ(RG_IRQ_STATUS, isr_src); \
                                                       \
    if (NULL != rf230_callback_handler) {              \
        rf230_callback_handler(isr_src);               \
    }                                                  \
} while (0)
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
#endif
/* EOF */
