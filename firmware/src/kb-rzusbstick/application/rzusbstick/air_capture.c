// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the AirCapture application
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
 * $Id: air_capture.c 41160 2008-04-29 18:26:40Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "vrt_mem.h"
#include "vrt_timer.h"
#include "led.h"
#include "at86rf230_registermap.h"
#include "rf230.h"
#include "usb_descriptors.h"
#include "usb_task.h"
#include "rzusbstickcommon.hh"
#include "air_capture.h"

//! \addtogroup applAirCapture
//! @{
/*================================= MACROS           =========================*/
/* State definitions for the AirCapture application. These are used in this file
 * only.
 */
#define AC_NOT_INITIALIZED   (0x01) //!< The air_capture_init function has not been called yet.
#define AC_IDLE              (0x02) //!< The AirCapture application initialized and ready.
#define AC_BUSY_CAPTURING    (0x04) //!< The AirCapture application is busy capturing frames.
#define AC_BUSY_JAMMING      (0x08) //!< The AirCapture application is busy jamming.
#define AC_BUSY_SCANNING     (0x10) //!< The AirCapture application is busy doing a scan.
#define AC_BUSY_TRANSMITTING (0x20) //!< The AirCapture application is busy sending a frame.
#define AC_TICK_PER_US       (2)   //!< Number of ticks per microsecond.

#define AC_SUPPORTED_INTERRUPT_MASK  (0x0C) //!< Only interrested in RX_START and TRX_END interrupts.
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*! \brief Variable holding the AirCapture application's internal state. */
static uint8_t ac_state = AC_NOT_INITIALIZED; 

/*! \brief Incremented once when a frame has been sent successfully to the PC. 
 *         Can be used for monitoring purposes.
 */
static uint16_t nmbr_of_frames = 0;

/*! \brief Incremented once when a frame has been reported, but it was not
 *         possible to store it. Can be used for monitoring purposes.
 */
static uint16_t nmbr_of_frames_missed = 0;

/* Variables used during callback processing. */
static uint8_t acdu_rssi; //!< Intermediate storage of received frames.
static uint32_t acdu_time_stamp; //!< Intermediate storage of received frame's timestamp.
static uint8_t ac_unknown_isr; //!< Incremented each time an unknown interrupt event is received by the AirCapture.

/* Variables used for fifo handling and control. */
static acdu_t *acdu_fifo[AC_ACDU_FIFO_SIZE]; //!< Queue of pointers to the allocated ACDUs.
static uint8_t fifo_head; //!< Variable storing head index in the fifo.
static uint8_t fifo_tail; //!< Variable storing tail index in the fifo.
static uint8_t items_used; //!< Variable storing number of items stored in the fifo.
static uint8_t items_free; //!< Variable storing number of items unused in fifo.

/* Variables used when doing USB transactions. */
static uint8_t bytes_left; //!< Bytes left to send of current frame.
static uint8_t packets_left; //!< Number of packets left in the transaction.
static uint8_t *data_ptr; //!< Pointer current byte to send.

/*! \brief Frame with randomized data. Used by the jammer. */
const PROGMEM_DECLARE(static uint8_t jammer_frame[127]) = {                   \
                        186,38,120,91,206,116,184,22,42,239,243,204,139,78, \
                        83,10,226,215,183,60,86,76,181,102,219,30,87,238,   \
                        230,244,67,26,6,223,205,159,134,62,138,121,58,4,9,  \
                        124,31,187,18,160,119,155,64,252,0,173,49,111,154,  \
                        166,158,21,13,108,68,112,53,240,100,214,126,72,61,  \
                        80,98,47,198,48,231,96,248,220,92,95,8,195,185,19,  \
                        168,190,233,122,129,101,188,210,46,85,229,144,247,  \
                        167,123,194,193,234,74,174,147,242,255,179,197,103, \
                        57,152,73,5,44,63,56,141,211,202,45,224,178,0,0};
/*================================= PROTOTYPES       =========================*/
static bool init_rf(void);
static void air_capture_callback(uint8_t isr_event);
static void air_capture_scan_callback(uint8_t isr_event);
static void air_capture_transmission_callback(uint8_t isr_event);

/*! \brief This function is used to initialize the RF230 radio transceiver to be
 *         used for capturing.
 *
 *  \retval true The radio transceiver was started successfully.
 *  \retval false The radio transceiver could not be started.
 */
static bool init_rf(void) {
    (bool)rf230_init();
    
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
    } else {
        
        /* Enable automatic CRC generation and set the ISR mask. */
        rf230_subregister_write(SR_CLKM_SHA_SEL, 0);
        rf230_subregister_write(SR_CLKM_CTRL, 0);
        rf230_subregister_write(SR_TX_AUTO_CRC_ON, 1);
        rf230_register_write(RG_IRQ_MASK, AC_SUPPORTED_INTERRUPT_MASK);
        
        RF230_ENABLE_TRX_ISR();
        
        rf230_init_status = true;
    }
    
    return rf230_init_status;
}


bool air_capture_init(void) {
    /* Initialize local variable. */
    nmbr_of_frames        = 0;
    nmbr_of_frames_missed = 0;
    acdu_rssi             = 0;
    acdu_time_stamp       = 0;
    ac_unknown_isr        = 0;
    
    fifo_head  = 0;
    fifo_tail  = 0;
    items_used = 0;
    items_free = AC_ACDU_FIFO_SIZE;
    
    bytes_left = 0;
    packets_left = 0;
    data_ptr = NULL;
    
    if (true != init_rf()) {
        goto init_failed;
    }
    
    /* Set-up the fifo of ACDUs: allocate memory and store the pointers in the FIFO. */
    for (uint8_t i = 0; i < AC_ACDU_FIFO_SIZE; i++) {
        
        acdu_t *acdu = (acdu_t *)vrt_mem_alloc(sizeof(acdu_t));
        
        if (NULL == acdu) {
            goto init_failed;
        } else {
            acdu_fifo[i] = acdu;
        }
    }
    
    ac_state = AC_IDLE;
    return true;
    
    
    
    /* Following label defines what to do if if the initialization fails. */
    init_failed:
    
    
    /* Disable the radio transceiver and release the memory. */
    rf230_deinit();
    for (uint8_t i = 0; (i < AC_ACDU_FIFO_SIZE); i++) {
        vrt_mem_free(acdu_fifo[i]);
        acdu_fifo[i] = (acdu_t *)NULL;
    }
    
    return false;
}


void air_capture_deinit(void) {
    /* Ensure that the AirCapture application has been running and memory allocated
     * before it can be teared down.
     */
    if (AC_NOT_INITIALIZED != ac_state) {
        /* Free allocated memory. */
        for (uint8_t i = 0; (i < AC_ACDU_FIFO_SIZE); i++) {
            vrt_mem_free(acdu_fifo[i]);
            acdu_fifo[i] = (acdu_t *)NULL;
        }
        
        /* Deinit the radio transceiver and set the internal status variable to
         * reflect the new state.
         */
        rf230_deinit();
        ac_state = AC_NOT_INITIALIZED;
    } // END: if (AC_NOT_INITIALIZED != ac_state) ...
}

/* This function must be called periodically to get captured data from the 
 * RF230 device driver over the USB interface to the connected PC. Or scan data 
 * to the PC.
 */
void air_capture_task(void) {
    /* The air_capture_task function has two main function to fullfill:
     * - If the ac_state equals AC_BUSY_CAPTURING it will send AirCapture data if available.
     * - If the ac_state equals AC_BUSY_SCANNING it will send scan data if available.
     */
    if (AC_BUSY_CAPTURING == ac_state) { 
        /* Check if there is data to be transmitted. */
        if (0 == items_used) {
            return;
        }
        
        /* Select the event EP. */
        UENUM = EP_EVENT;
        
        /* Check that it is possible to fill at least one bank (64 bytes) in the
         * DPRAM.
         */
        if ((UEINTX & (1 << TXINI)) != (1 << TXINI)) {
            return;
        }
        
        /* ACK TX_IN. */
        UEINTX &= ~(1 << TXINI);
        
        /* If there is no acdu allocated, but data left in the AirCapture fifo.
         * One acdu must be taken from the fifo's tail, however the tail's
         * position will not be updated until the complete acdu is written. This
         * scheme is used to prevent data loss in the event pump, with varying
         * poll frequency from the PC.
         */
        if (0 == bytes_left) {
            if (0 == packets_left) {
                /* Get new acdu, since all bytes and all packets in transaction
                 * are sent. The calculations below assumes an EP size of 64 bytes.
                 * The driver has been implemented for USB bulk transmission.
                 */
                data_ptr = (uint8_t *)acdu_fifo[fifo_tail];
                bytes_left = *(data_ptr + 1);
                packets_left = (uint8_t)(((uint16_t)(bytes_left + 64 - 1)) >> 6);
                if (0 == (bytes_left % 64)) { packets_left++ ; }
            } else if (1 == packets_left) {
                /* Send Zero Length Packet and then update tail pointer. */
                UEINTX &= ~(1 << FIFOCON);
                
                /* Update FIFO tail. */
                ENTER_CRITICAL_REGION();
                
                if ((AC_ACDU_FIFO_SIZE - 1) == fifo_tail) {
                    fifo_tail = 0;
                } else {
                    fifo_tail++;
                } // END: if ((AC_ACDU_FIFO_SIZE - 1) == fifo_tail) ...
                
                items_used--;
                items_free++;
                
                nmbr_of_frames++;
                
                LEAVE_CRITICAL_REGION();
                
                return;
            } else {
                /* Turn the orange LED on to signal a level 2. error. The 
                 * application should be restarted.
                 */
                LED_ORANGE_ON();
            }// END: if (0 == packets_left) ...
        } // END: if ((0 == bytes_left) ...
        
        /* At least one byte to send. */
        do {
            UEDATX = *data_ptr;
            data_ptr++;
            bytes_left--;
        } while ((0 != bytes_left) && ((UEINTX & (1 << RWAL)) == (1 << RWAL)));
        
        /* Either 64 bytes or the last byte in a packet is written. */
        packets_left--;
        
        /* ACK writing to the bank. */
        UEINTX &= ~(1 << FIFOCON);
        
        /* Check if it is time to move tail. */
        if ((0 == bytes_left) && (0 == packets_left)) {
            ENTER_CRITICAL_REGION();
                
            if ((AC_ACDU_FIFO_SIZE - 1) == fifo_tail) {
                fifo_tail = 0;
            } else {
                fifo_tail++;
            } // END: if ((AC_ACDU_FIFO_SIZE - 1) == fifo_tail) ...
                
            items_used--;
            items_free++;
                
            nmbr_of_frames++;
                
            LEAVE_CRITICAL_REGION(); 
        } // END: if ((0 == bytes_left) && (0 == packets_left)) ...
    } else if (AC_BUSY_SCANNING == ac_state) {
        /* Not implemented yet, so only a dummy is written for debug purposes. */
        usb_ep_write(EP_EVENT, 8, (uint8_t *)"SCANSCAN");
    } else {
        /* Do nothing. Any other state than AC_BUSY_CAPTURING and AC_BUSY_SCANNING
         * does not require any handling.
         */
    } // END: if (AC_BUSY_CAPTURING != ac_state) ...
}

/* This function will set new channel for the radio transceiver to work on. */
bool air_capture_set_channel(uint8_t channel) {
    
    /* Perform sanity checks to see if it is  possible to run the function. */
    if (AC_IDLE != ac_state) { return false; }
    if ((channel < AC_MIN_CHANNEL) || (channel > AC_MAX_CHANNEL)) { return false; }
    
    /* Fix for timing issue with setting channel immediately after setting CMD_MODE_AC - JLW */
    /* Could be that we were sleeping before we got here. */
    delay_us(TIME_SLEEP_TO_TRX_OFF);

    /* Force TRX_OFF mode and wait for transition to complete. */
    rf230_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    delay_us(TIME_P_ON_TO_TRX_OFF);
    
    /* Set channel and verify. */
    bool ac_set_channel_status = false;
    if (TRX_OFF == rf230_subregister_read(SR_TRX_STATUS)) {
        /* Set the new channel and verify. */
        rf230_subregister_write(SR_CHANNEL, channel);
        if (channel != rf230_subregister_read(SR_CHANNEL)) {
        } else {
            ac_set_channel_status = true;
        } // END: if (channel != rf230_subregister_read(SR_CHANNEL)) ...
    } // END: if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) ...
    
    return ac_set_channel_status;
}

/* This function will try to start capturing frames from the air. */
bool air_capture_open_stream(void) {
    /* Check that the AirCapture application is initialized and not busy. */
    if (AC_IDLE != ac_state) { return false; }
    
    /* Initialize the frame pool in the RF230 device driver and set the radio 
     * transceiver in receive mode. 
     */
    rf230_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    delay_us(TIME_P_ON_TO_TRX_OFF);
    
    bool ac_open_stream_status = false;
    if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) { 
    } else {
        /* Do transition from TRX_OFF to RF_ON. */
        rf230_subregister_write(SR_TRX_CMD, RX_ON);
        delay_us(TIME_TRX_OFF_TO_PLL_ACTIVE);
        
        /* Verify that the state transition to RX_ON was successful. */
        if (RX_ON != rf230_subregister_read(SR_TRX_STATUS)) {
        } else {
            /* Reset the event FIFO. */
            ENTER_CRITICAL_REGION();
            
            fifo_head = 0;
            fifo_tail = 0;
            items_used = 0;
            items_free = AC_ACDU_FIFO_SIZE;
            
            bytes_left = 0;
            packets_left = 0;
            data_ptr = NULL;
            
            nmbr_of_frames = 0;
            
            LEAVE_CRITICAL_REGION();
            
            /* Set callback for captured frames and update AirCapture status. */
            rf230_set_callback_handler(air_capture_callback);
            ac_state = AC_BUSY_CAPTURING;
            ac_open_stream_status = true;
        } // END: if (RX_ON != rf230_subregister_read(SR_TRX_STATUS)) ...
    } // END: if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) ...
    
    return ac_open_stream_status;
}

/* This function stops the radio transceiver from capturing frames. */
bool air_capture_close_stream(void) {
    /* Perform sanity checks to see if it is  possible to run the function. */
    if (AC_BUSY_CAPTURING != ac_state) { return false; }
    
    /* Close stream. */
    rf230_clear_callback_handler();
    rf230_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    delay_us(TIME_P_ON_TO_TRX_OFF);
    
    /* Verify that the TRX_OFF state was entered. */
    bool ac_close_stream_status = false;
    if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) { 
    } else {
        ac_state = AC_IDLE;
        ac_close_stream_status = true;
    } // END: if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) ...
    
    return ac_close_stream_status;
}

/* This function is used to start a scan on the current channel. */
bool air_capture_start_channel_scan(uint8_t scan_type, uint8_t scan_duration) {
    /* Not implemented yet. Below code is just to prevent generation of warning
     * in the compiler.
     */
    rf230_set_callback_handler(air_capture_scan_callback);
    rf230_clear_callback_handler();
    return false;
}

/* This function will stop the scan even if it is not yet finished. */
bool air_capture_stop_channel_scan(void) {
    /* Perform sanity checks to see if it is  possible to run the function. */
    if (AC_BUSY_SCANNING != ac_state) { return false; }
    
    /* Stop scan. */
    rf230_clear_callback_handler();
    rf230_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    delay_us(TIME_P_ON_TO_TRX_OFF);
    
    /* Verify that the TRX_OFF state was entered. */
    bool ac_stop_scan_status = false;    
    if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) { 
    } else {
        ac_state = AC_IDLE;
        ac_stop_scan_status = true;
    } // END: if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) ...
    
    return ac_stop_scan_status;
}

/* This function will inject an user defined frame onto the current channel. */
uint8_t air_capture_inject_frame(uint8_t length, uint8_t *frame) {

    /* Perform sanity checks to see if it is  possible to run the function. */
    if ((0 == length) || (RF230_MAX_FRAME_LENGTH < length)) { return 1; }
    if (NULL == frame) { return 2; }
    
    /* Check that the AirCapture application is initialized and not busy. */
    if (AC_IDLE != ac_state) { return 3; }
    
    /* Check that the radio transceiver is in TRX_OFF. */
    if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) { return 4; }
    
    /* Go to PLL_ON and send the frame. */
    rf230_subregister_write(SR_TRX_CMD, CMD_PLL_ON);
    delay_us(TIME_TRX_OFF_TO_PLL_ACTIVE);
    
    bool ac_inject_frame_status = false;
    
    /* Verify that the PLL_ON state was entered. */
    if (PLL_ON != rf230_subregister_read(SR_TRX_STATUS)) { 
    } else {
        rf230_set_callback_handler(air_capture_transmission_callback);
        
        /* Send frame with pin start. */
        rf230_set_slptr_high();
        rf230_set_slptr_low();
        rf230_frame_write(length, frame);
        
        /* Update state information. */
        ac_state = AC_BUSY_TRANSMITTING;
        ac_inject_frame_status = true;
    } // END: if (PLL_ON != rf230_subregister_read(SR_TRX_STATUS)) ...
	
	if (ac_inject_frame_status == true) {
		return 0;
	} else {
	    	return 5;
	}
}

/* This function starts the jammer. */
bool air_capture_jammer_on(void) {
    /* Check that the AirCapture application is initialized and not busy. */
    if (AC_IDLE != ac_state) { return false; }
    
    /* Check that the radio transceiver is in TRX_OFF. */
    if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) { return false; }
    
    /* Download set of randomized data and start PBRS mode. See Appendix A in the
     * RF230's datasheet for detailed instructions on how to use the different
     * internal test modes.
     */
    rf230_frame_write_P(sizeof(jammer_frame), jammer_frame);
    rf230_register_write(0x36, 0x0F);
    rf230_register_write(0x3D, 0x00);
    rf230_set_tst_high();
    
    /* Do state transition to PLL_ON and verify. */
    rf230_subregister_write(SR_TRX_CMD, CMD_PLL_ON);
    delay_us(TIME_TRX_OFF_TO_PLL_ACTIVE);
    
    bool ac_jammer_on_status = false;
    if (PLL_ON != rf230_subregister_read(SR_TRX_STATUS)) { 
        /* Reset the radio transceiver. */
        (bool)rf230_init();
    } else {
        /* Start PBRS mode and update AirCapture status. */
        rf230_register_write(0x02, 0x02);
        ac_state = AC_BUSY_JAMMING;
        ac_jammer_on_status = true;
    } // END: if (PLL_ON != rf230_subregister_read(SR_TRX_STATUS)) ...
    
    return ac_jammer_on_status;
}

/* This function stops the jammer. */
bool air_capture_jammer_off(void) {
    /* Perform sanity checks to see if it is  possible to run the function. */
    if (AC_BUSY_JAMMING != ac_state) { return false; }
    
    /* Stop scan: Reset the radio transceiver by re-initializing it. */ 
    bool ac_jammer_off_status = false;
    if (true != rf230_init()) {
    } else {
        ac_state = AC_IDLE;
        ac_jammer_off_status = true;
    } // END: if (TRX_OFF != rf230_subregister_read(SR_TRX_STATUS)) ...
    
    return ac_jammer_off_status;
}

/*! \brief This is an internal callback function that is used to handle the 
 *         reception of frames in AirCapture mode.
 *
 *  This is a callback function from the RF230 Device Driver interrupt system,
 *  so herein one is protected from other interrupts etc. It is therefore safe
 *  to use the function-like macros defined in the RF230 Device Driver header file.
 *  The implementation has been optimized for speed. One of the schemes used is to
 *  exploit that it takes 16 cycles to clock out one byte over the SPI as master at
 *  the highest possible speed.
 *
 *  \param[in] isr_event Event signaled by the radio transceiver.
 */
static void air_capture_callback(uint8_t isr_event) {
    /* Mask the two different events interesting in this application:
     * - TRX_END: Complete frame received.
     * - RX_START: SFD of new frame received.
     */
    if (RF230_TRX_END_MASK == (isr_event & RF230_TRX_END_MASK)) {
    /* End of frame indicated. Upload it if there is packets in the pool left. */
        if (0 == items_free) { nmbr_of_frames_missed++; return; }
        
        RF230_SS_LOW();
        
        SPDR = RF230_TRX_CMD_FR; // Send Frame Read Command.
        
        /* Get ACDU while waiting for SPI to finish data transmission. */
        acdu_t *this_acdu = acdu_fifo[fifo_head];
        if ((AC_ACDU_FIFO_SIZE - 1) == fifo_head) {
            fifo_head = 0;
        } else {
            fifo_head++;
        } // END: if ((AC_ACDU_FIFO_SIZE - 1) == fifo_head) ...
        
        items_used++;
        items_free--;
        
        /* Poll to ensure that data was sent. */
        RF230_WAIT_FOR_SPI_TX_COMPLETE();
        uint8_t frame_length = SPDR;
        
        SPDR = frame_length;
        
        /* Set ACDU ID and RSSI while waiting for SPI to finish data transmission. */
        this_acdu->ac_id = EVENT_STREAM_AC_DATA;
        this_acdu->rssi = acdu_rssi;
        
        uint8_t *frame = this_acdu->frame;
        
        /* Poll to ensure that data was sent. */
        RF230_WAIT_FOR_SPI_TX_COMPLETE();
        frame_length = (SPDR & 0x7F); // Real frame length returned from the radio transceiver. And with mask to avoid overflow.
        
        /* Calculate the length of the stored ACDU in bytes. The mystical 10 bytes
         * added is the length of the non-variable length fields in an ACDU.
         */
        this_acdu->length = frame_length + 10;
        *frame = frame_length;
        frame++;
        
        frame_length++; // Add one byte to the length for LQI.
        
        SPDR = frame_length;
        
        /* Set ACDU timestamp while waiting for SPI to finish data transmission. */
        this_acdu->time_stamp = acdu_time_stamp;
        
        /* Poll to ensure that data was sent. */
        RF230_WAIT_FOR_SPI_TX_COMPLETE();   
        
        /* Upload frame. */
        do {
            uint8_t const temp_data = SPDR;
            SPDR = temp_data; // Any data will do, and tempData is readily available. Saving cycles.
            
            *frame = temp_data;      
                
            frame++;
            frame_length--;
            RF230_WAIT_FOR_SPI_TX_COMPLETE();
        } while (0 != frame_length);

        RF230_SS_HIGH();
        
        /* Read CRC flag. */
        RF230_QUICK_SUBREGISTER_READ(RG_PHY_RSSI , 0x80, 7, (this_acdu->crc));
    } else if (RF230_RX_START_MASK == (isr_event & RF230_RX_START_MASK)) {
        /* Start of frame indicated. Read RSSI and timestamp the frame. */
        acdu_time_stamp = vrt_timer_get_tick_cnt() / AC_TICK_PER_US;
        RF230_QUICK_SUBREGISTER_READ(0x06, 0x1F, 0, acdu_rssi);
    } else {
        /* Unknow interrupt occured for this mode. No special handling will
         * be done, except incrementing a statistical variable.
         */
        ac_unknown_isr++;
    } // END: if (RF230_TRX_END_MASK == (isr_event & RF230_TRX_END_MASK)) ...
}

/*! \brief This is an internal callback function that is used to handle the 
 *         reception of frames during channel scans.
 *
 *  This is a callback function from the RF230 Device Driver interrupt system,
 *  so herein one is protected from other interrupts etc. It is therefore safe
 *  to use the function-like macros defined in the RF230 Device Driver header file.
 *  The implementation has been optimized for speed. One of the schemes used is to
 *  exploit that it takes 16 cycles to clock out one byte over the SPI as master at
 *  the highest possible speed.
 *
 *  \param[in] isr_event Event signaled by the radio transceiver.
 */
static void air_capture_scan_callback(uint8_t isr_event) {
    /* Not implemented yet. */
}
                            
/*! \brief This is an internal callback function that is used to handle the 
 *         signal from the radio transceiver when a frame has been injected.
 *
 *  This is a callback function from the RF230 Device Driver interrupt system,
 *  so herein one is protected from other interrupts etc. It is therefore safe
 *  to use the function-like macros defined in the RF230 Device Driver header file.
 *  The implementation has been optimized for speed. One of the schemes used is to
 *  exploit that it takes 16 cycles to clock out one byte over the SPI as master at
 *  the highest possible speed.
 *
 *  \param[in] isr_event Event signaled by the radio transceiver.
 */
static void air_capture_transmission_callback(uint8_t isr_event) {
    if (RF230_TRX_END_MASK == (isr_event & RF230_TRX_END_MASK)) {
        RF230_QUICK_CLEAR_ISR_CALLBACK();
    
        /* Force radio transceiver to TRX_OFF mode and set AirCapture state to AC_IDLE. */
        RF230_QUICK_SUBREGISTER_WRITE(0x02, 0x1f, 0, CMD_FORCE_TRX_OFF);
        delay_us(TIME_CMD_FORCE_TRX_OFF);
        ac_state = AC_IDLE;
    } // END: if (RF230_TRX_END_MASK == (isr_event & RF230_TRX_END_MASK))...
}
//! @}
/*EOF*/
