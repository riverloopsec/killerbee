// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Specific implementation of the TAT for the AT86RF230 radio transceiver
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
 * $Id: tat_rf230.c 41483 2008-05-09 12:55:46Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "compiler.h"
#include "at86rf230_registermap.h"
#include "rf230.h"
#include "tat.h"
#include "tat_conf.h"
#include "vrt_kernel.h"
/*================================= MACROS           =========================*/
#define RF230_REVB                      (2)
#define RF230_SUPPORTED_MANUFACTURER_ID (31)
#define RF230_SUPPORTED_INTERRUPT_MASK  (0x0C)
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
static uint8_t transmission_status = 0xFF;    //!< Status flag storing the status of the last frame transmission.
static uint8_t frame_length = 0;              //!< Length of the last received frame.
static volatile bool waiting_for_ack = false; //!< Flag that will be set true when an ACK is received after frame transmission.
static tat_cb_handler_t tx_cb = NULL;         //!< Pointer to function that will be posted to the VRT_KERNEL when a transmission is complete.
static tat_cb_handler_t rx_cb = NULL;         //!< Pointer to function that will be posted to the VRT_KERNL when a new frame is received.
/*================================= PROTOTYPES       =========================*/
static void tat_trx_end_cb(uint8_t trx_isr_event);


bool tat_init(void) {
    /* Initialize local varaibles. */
    transmission_status = 0xFF;
    frame_length = 0;
    waiting_for_ack = false;
    tx_cb = NULL;
    rx_cb = NULL;
        
    /* Initialize the low level driver for the radio transceiver. */
    if (true != rf230_init()) {
        return false;
    }
    
    /* Take the radio transceiver to P_ON state. */
    delay_us(TIME_TO_ENTER_P_ON);
    
    rf230_set_tst_low();
    rf230_set_rst_low();
    rf230_set_slptr_low();
    delay_us(TIME_RESET);    
    rf230_set_rst_high();
    
    /* Could be that we were sleeping before we got here. */
    delay_us(TIME_SLEEP_TO_TRX_OFF);
    
    /* Could be that we were sleeping before we got here. */
    delay_us(TIME_SLEEP_TO_TRX_OFF);
    
    /* Force transition to TRX_OFF and verify. */
    rf230_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    delay_us(TIME_P_ON_TO_TRX_OFF);
    
    bool rf230_init_status = false;
    if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) {
    } else if (RF230_REVB != rf230_register_read(RG_VERSION_NUM)) {
    } else if (RF230_SUPPORTED_MANUFACTURER_ID != rf230_register_read(RG_MAN_ID_0)) {
    } else {
        /* Set callback handler for the interrupts signaled by the radio transceiver. */
        rf230_set_callback_handler(tat_trx_end_cb);
        
        /* Enable automatic CRC generation and set the ISR mask. */
        rf230_subregister_write(SR_CLKM_SHA_SEL, 0);
        rf230_subregister_write(SR_CLKM_CTRL, 0);
        rf230_subregister_write(SR_TX_AUTO_CRC_ON, 1);
        rf230_register_write(RG_IRQ_MASK, RF230_SUPPORTED_INTERRUPT_MASK);
        
        RF230_ENABLE_TRX_ISR();
        
        rf230_init_status = true;
    }
    
    return rf230_init_status;
}


void tat_deinit(void) {
    rf230_clear_callback_handler();
    
    /* Take the radio transceiver to sleep and then deinit the driver for the
     * radio transceiver.
     */
    tat_go_to_sleep();
    rf230_deinit();
}


void tat_reset(void) {
    /* Make sure that the device is awake. */
    (bool)tat_wake_up();
    
    /* Make sure that TRX_OFF is entered. */
    do {
        rf230_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    } while (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS));
    
    frame_length = 0;
    waiting_for_ack = false;
}


void tat_go_to_sleep(void) {
    /* Reset the waiting for ack flag. Should only be set in the period between 
     * a frame being sent and until the associated ACK arrives (BUSY_TX_ARET).
     */
    waiting_for_ack = false;
    
    if (1 == rf230_get_slptr()) { return; } // Check that we are not already sleeping.
    if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) {
        do {
            rf230_subregister_write(SR_TRX_CMD, CMD_TRX_OFF);
        } while (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS));
    }
    
    /* Put the radio transceiver to sleep and wait until sleep state has been entered. */
    rf230_set_slptr_high(); // Go to SLEEP from TRX_OFF.
    delay_us(TIME_TRX_OFF_TO_SLEEP);
}


bool tat_wake_up(void) {
    /* Reset the waiting for ack flag. Should only be set in the period between 
     * a frame being sent and until the associated ACK arrives (BUSY_TX_ARET).
     */
    waiting_for_ack = false;
    
    /* Check if device is sleeping or not. */
    if (0 == rf230_get_slptr()) {
        return true;
    }
    
    /* Devcie was sleeping: Wake it up. */
    rf230_set_slptr_low(); // Go to TRX_OFF from SLEEP.
    delay_us(2000); // Wait for state transition to complete.
    
    /* Verify that the device has entered the TRX_OFF state. */
    if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) {
        return false;
    } else {
        return true;
    }
}


uint16_t tat_get_pan_id(void) {
    /* Not implemented yet. */
    return 0;
}


void tat_set_pan_id(uint16_t pan_id) {
    uint8_t pan_id_byte = ((pan_id >> 8*0) & 0xFF); // Extract short_address_7_0.
    rf230_register_write(RG_PAN_ID_0, pan_id_byte);
    
    pan_id_byte = ((pan_id >> 8*1) & 0xFF); // Extract short_address_15_8.
    rf230_register_write(RG_PAN_ID_1, pan_id_byte);
}


uint16_t tat_get_short_address(void) {
    /* Not implemented yet. */
    return 0;
}


void tat_set_short_address(uint16_t short_address) {
    uint8_t address_byte = ((short_address >> 8*0) & 0xFF); // Extract short_address_7_0.
    rf230_register_write(RG_SHORT_ADDR_0, address_byte);
    
    address_byte = ((short_address >> 8*1) & 0xFF); // Extract short_address_15_8.
    rf230_register_write(RG_SHORT_ADDR_1, address_byte);
}


uint64_t tat_get_ieee_address(void) {
    /* Not implemented yet. */
    return 0;
}


void tat_set_ieee_address(const uint8_t *ieee_address) {
    rf230_register_write(RG_IEEE_ADDR_0, *ieee_address++);
    rf230_register_write(RG_IEEE_ADDR_1, *ieee_address++);
    rf230_register_write(RG_IEEE_ADDR_2, *ieee_address++);
    rf230_register_write(RG_IEEE_ADDR_3, *ieee_address++);
    rf230_register_write(RG_IEEE_ADDR_4, *ieee_address++);
    rf230_register_write(RG_IEEE_ADDR_5, *ieee_address++);
    rf230_register_write(RG_IEEE_ADDR_6, *ieee_address++);
    rf230_register_write(RG_IEEE_ADDR_7, *ieee_address++);
}


void tat_set_pan_coordinator(bool iam_pan_coordinator) {
    if (true != iam_pan_coordinator) {
        rf230_subregister_write(SR_I_AM_COORD, 0);
    } else {
        rf230_subregister_write(SR_I_AM_COORD, 1);
    }
}


bool tat_send_frame(uint8_t length, const uint8_t *frame) {
    /* Verify that the function parameters are sane. */
    if (0 == length) {
        return false;
    }
    
    if (NULL == frame) {
        return false;
    }
    
    ENTER_CRITICAL_REGION();
    
    /* Check that we are not busy transmitting a frame already. */
    if (true == waiting_for_ack) {
        LEAVE_CRITICAL_REGION();    
        return false;
    }
    
    /*
     * Toggle the SLP_TR pin to quickly initiate Initiate
     * frame transmission.
     */
    rf230_set_slptr_high();
    rf230_set_slptr_low();

    LEAVE_CRITICAL_REGION();
    
    rf230_frame_write(length, frame);
    waiting_for_ack = true;
    
    return true;
}


bool tat_read_frame(uint8_t length, uint8_t *rd_buffer) {
    /* Perform sanity check on function parameters. */
    if (0 == length) {
        return false;
    }
    
    if (NULL == rd_buffer) {
        return false;
    }
    
    /* Upload farme from the frame buffer. */
    rf230_frame_read(length, rd_buffer);
    
    return true;
}


uint8_t tat_get_state(void) {
    return rf230_subregister_read(SR_TRX_STATUS);
}


bool tat_set_state(uint8_t new_state) {
    /* Reset the waiting for ack flag. Should only be set in the period between 
     * a frame being sent and until the associated ACK arrives (BUSY_TX_ARET).
     */
    waiting_for_ack = false;
    
    /*Check function paramter and current state of the radio transceiver.*/
    if (!((new_state == TRX_OFF ) || (new_state == RX_ON) || (new_state == PLL_ON) || 
        (new_state == RX_AACK_ON ) || (new_state == TX_ARET_ON ))) {
            return false;
    }
    
    /* Check if the radio transceiver is sleeping. */
    if (rf230_get_slptr() == 1) {
        return false;
    }
    
    uint8_t original_state = rf230_subregister_read(SR_TRX_STATUS);
    
    if ((original_state == BUSY_RX ) || (original_state == BUSY_TX) || 
        (original_state == BUSY_RX_AACK) || (original_state == BUSY_TX_ARET)) { return false; }
    
    if (new_state == original_state) { return true; }
                        
    //At this point it is clear that the requested new_state is:
    //TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON or TX_ARET_ON.
                
    //The radio transceiver can be in one of the following states:
    //TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON, TX_ARET_ON.
    if(new_state == TRX_OFF){
        rf230_set_slptr_low();
        delay_us(TIME_NOCLK_TO_WAKE);
        rf230_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
        delay_us(TIME_CMD_FORCE_TRX_OFF);
    } else {
        
        //It is not allowed to go from RX_AACK_ON or TX_AACK_ON and directly to
        //TX_AACK_ON or RX_AACK_ON respectively. Need to go via RX_ON or PLL_ON.
        if ((new_state == TX_ARET_ON) && (original_state == RX_AACK_ON)) {
            
            //First do intermediate state transition to PLL_ON, then to TX_ARET_ON.
            //The final state transition to TX_ARET_ON is handled after the if-else if.
            rf230_subregister_write(SR_TRX_CMD, PLL_ON);
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        } else if ((new_state == RX_AACK_ON) && (original_state == TX_ARET_ON)) {
            
            //First do intermediate state transition to RX_ON, then to RX_AACK_ON.
            //The final state transition to RX_AACK_ON is handled after the if-else if.
            rf230_subregister_write(SR_TRX_CMD, RX_ON);
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }
            
        //Any other state transition can be done directly.    
        rf230_subregister_write(SR_TRX_CMD, new_state);
        
        //When the PLL is active most states can be reached in 1us. However, from
        //TRX_OFF the PLL needs time to activate.
        if (original_state == TRX_OFF) {
            delay_us(TIME_TRX_OFF_TO_PLL_ACTIVE);
        } else {
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }
    }
        
    /*Verify state transition.*/
    if(rf230_subregister_read(SR_TRX_STATUS) == new_state) { 
        return true; 
    } else {
        return false;
    }
}


uint8_t tat_get_channel(void) {
    return 0;
}


bool tat_set_channel(uint8_t channel) {
    /* Check that the selected channel is within bounds. */
    if ((channel < TAT_MIN_CHANNEL) || (channel > TAT_MAX_CHANNEL)) {
        return false;
    }
    
    /*Set new operating channel.*/        
    rf230_subregister_write(SR_CHANNEL, channel);
    
    bool set_channel_status = false;
    /* Verify that the channel was set. */
    if (channel == rf230_subregister_read(SR_CHANNEL)) {
        set_channel_status = true;
    }
    
    return set_channel_status;
}


void tat_set_tx_callback_handler(tat_cb_handler_t handler) {
    ENTER_CRITICAL_REGION();
    tx_cb = handler;
    LEAVE_CRITICAL_REGION();
}


tat_cb_handler_t tat_get_tx_callback_handler(void) {
    tat_cb_handler_t handler = NULL;
    
    ENTER_CRITICAL_REGION();
    handler = tx_cb;
    LEAVE_CRITICAL_REGION();
    
    return handler;
}


void tat_clear_tx_callback_handler(void) {
    ENTER_CRITICAL_REGION();
    tx_cb = NULL;
    LEAVE_CRITICAL_REGION();
}


void tat_set_rx_callback_handler(tat_cb_handler_t handler) {
    ENTER_CRITICAL_REGION();
    rx_cb = handler;
    LEAVE_CRITICAL_REGION();
}


tat_cb_handler_t tat_get_rx_callback_handler(void) {
    tat_cb_handler_t handler = NULL;
    
    ENTER_CRITICAL_REGION();
    handler = rx_cb;
    LEAVE_CRITICAL_REGION();
    
    return handler;
}


void tat_clear_rx_callback_handler(void) {
    ENTER_CRITICAL_REGION();
    rx_cb = NULL;
    LEAVE_CRITICAL_REGION();
}


static void tat_trx_end_cb(uint8_t trx_isr_event) {
    /* Verify that this is an TRX_END interrupt. */
    if (RF230_TRX_END_MASK != (trx_isr_event & RF230_TRX_END_MASK)) {
        return;
    }
    
    /* Check if this interrupt event is associated with a frame transmission. If
     * so, store the transmission status and take the radio transceiver to TRX_OFF. On
     * the other hand, if the interrupt event is associated with a frame reception the
     * radio transceiver must be taken to TRX_OFF immidiately to protect the frame buffer.
     */
    
    if (false == waiting_for_ack) {
        /* RX_AACK_ON state. */
        
        /* Check if there is an event handler/callback assigned for this state. */
        if (NULL == rx_cb) {
            return;
        }
        
        /* Read frame length and check if it is valid. */
        RF230_QUICK_READ_FRAME_LENGTH(frame_length);
        if (frame_length < TAT_MIN_FRAME_LENGTH) {
            return;
        }
        
        /* Frame length is valid, so the radio transceiver is taken to a TX state
         * to protect the frame buffer. It will be up to the next higher layer to
         * put the radio transceiver either to sleep or back in RX_AACK_ON mode,
         * or to sleep.
         */
        RF230_QUICK_SUBREGISTER_WRITE(0x02, 0x1F, 0, CMD_PLL_ON);
        
        /* Post to the VRT_KERNEL that the associated event must be executed. */
        if (0 == VRT_GET_ITEMS_FREE()) {
            VRT_EVENT_MISSED();
            return;
        }
        
        vrt_quick_post_event(rx_cb, (void*)(&frame_length));
    } else {
        
        /* TX_ARET_ON state. */
        
        waiting_for_ack = false;
        
        /* Read transaction status and go to TRX_OFF. It is up to the next higher
         * layer to put the radio transceiver in the next state.
         */
        RF230_QUICK_SUBREGISTER_READ(0x02, 0xE0, 5, transmission_status);
        RF230_QUICK_SUBREGISTER_WRITE(0x02, 0x1F, 0, CMD_FORCE_TRX_OFF);
        
        /* Check if there is an event handler/callback assigned for this state. */
        if (NULL == tx_cb) {
            return;
        }
        
        /* Check if it is possible to post event. */
        if (0 == VRT_GET_ITEMS_FREE()) {
            VRT_EVENT_MISSED();
            return;
        }
        
        vrt_quick_post_event(tx_cb, (void*)(&transmission_status));
    }
}
/*EOF*/
