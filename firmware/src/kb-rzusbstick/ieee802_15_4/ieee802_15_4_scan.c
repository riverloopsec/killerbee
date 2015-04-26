// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the IEEE 802.15.4 MLME_SCAN primitive
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
 * $Id: ieee802_15_4_scan.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "compiler.h"
#include "vrt_kernel.h"
#include "vrt_kernel_conf.h"
#include "vrt_mem.h"

#include "at86rf230_registermap.h"
#include "rf230.h"
#include "tat.h"
#include "ieee802_15_4.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_msg_types.h"
#include "ieee802_15_4_pib.h"
#include "ieee802_15_4_internal_api.h"
/*================================= MACROS           =========================*/
/** Max beacon Order in beacon-enabled network */
#define BEACON_NETWORK_MAX_BO   (14)
#define ORPHANNOTIFICATION      (0x06)
#define BEACONREQUEST           (0x07)
/*! \brief Time it takes in symbols to perform one ED scan. This is the 140 us 
 *         plus overhead.
 */
#define ED_SCAN_TIME            (16)

#define BEACONREQUEST_CMD_SIZE (10)
#define ORPHANINDICATION_CMD_SIZE (20)
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function is executed when the scan timer expired.
 *
 *  \param[in] null This parameter is not used. Vill be NULL.
 */
static void scan_time_elapsed_callback(void *null);


/*! \brief Implements the ACTIVE scan specific part of the MLME-SCAN.request 
 *         primitive.
 *
 *  \param[in] ScanDuration Length of the scan in symbols.
 */
static bool do_active_scan(uint8_t ScanDuration);

/*! \brief Implements the PASSIVE scan specific part of the MLME-SCAN.request
 *         primitive.
 *
 *  \param[in] ScanDuration Length of the scan in symbols.
 */
static bool do_passive_scan(uint8_t ScanDuration);

/*! \brief Implements the ORPHAN scan specific part of the MLME-SCAN.request 
 *         primitive.
 *
 *  \param[in] ScanDuration Length of the scan in symbols.
 */
static bool do_orphan_scan(uint8_t ScanDuration);


bool ieee802_15_4_scan_request(mlme_scan_req_t* msr) {
    /* This primitive can only be executed with the IEEE 802.15.4 MAC's internal 
     * state.
     */
    if (IEEE_MAC_IDLE != IEEE802_15_4_GET_STATE()) { return false; }
    
    /* Perform sanity check on supplied set of parameters. */
    if (NULL == msr) { return false; }
    if (NULL == (msr->mlme_scan_confirm)) { return false; }
    
    uint8_t scan_type = msr->ScanType;
    uint8_t scan_duration = msr->ScanDuration;
    uint8_t scan_channel = msr->ScanChannel;
    
    if (scan_duration > BEACON_NETWORK_MAX_BO)  { return false; }
    
    /* Try to wake up the device and take it to TRX_OFF. */
    if (true != tat_wake_up()) {
        tat_go_to_sleep();
        return false;
    }
    
    /* Try to set the selected channel. */    
    if (MAC_SUCCESS != ieee802_15_4_set_channel(scan_channel)) {
        tat_go_to_sleep();
        return false;
    }
    
    /* Set callback and storage pointer before proceeding. These parameters are
     * stored in the internal IEEE 802.15.4 MAC storgae container (mac_param).
     */
    mac_param.scan.mlme_scan_confirm = msr->mlme_scan_confirm;
    mac_param.scan.msc = &(msr->msc);
    
    bool msr_status = false;
    
    
    if (MLME_SCAN_TYPE_ED == scan_type) {
        /* ED scan not supported at the moment. */
    } else if (MLME_SCAN_TYPE_ACTIVE == scan_type) {
        msr_status = do_active_scan(scan_duration);
    } else if (MLME_SCAN_TYPE_PASSIVE == scan_type) {
        msr_status = do_passive_scan(scan_duration);
    } else if (MLME_SCAN_TYPE_ORPHAN == scan_type) {
        msr_status = do_orphan_scan(scan_duration);
    } else {
        /* Unknown scan type. */
        tat_go_to_sleep();
        msr_status = false;
    }
    
    return msr_status;
}


static bool do_active_scan(uint8_t ScanDuration) {
    /* Try to allocate some memory to build the Beacon Request commando on. */
    uint8_t* beacon_request = IEEE802_15_4_GET_TX_BUFFER();

    /* Calculate scan duration in symbols. */
    mac_param.scan.scan_time = (1 << ScanDuration) + 1;
    mac_param.scan.scan_time *= aBaseSuperframeDuration;
    
    /* Store current PAN_ID as specified in the IEEE802.15.4 standard. */
    mac_param.scan.saved_pan_id = IEEE802_15_4_GET_PAN_ID();
    
    uint16_t fcf = FCF_SET_FRAMETYPE(FCF_FRAMETYPE_MAC_CMD) |
                   FCF_SET_DEST_ADDR_MODE(FCF_SHORT_ADDR)   |
                   FCF_SET_SOURCE_ADDR_MODE(FCF_NO_ADDR);

    uint8_t index = 0;
    beacon_request[index++] = ((fcf >> 8*0) & 0xFF);
    beacon_request[index++] = ((fcf >> 8*1) & 0xFF);

    /* sequence number */
    beacon_request[index++] = IEEE802_15_4_GET_DSN();

    /* dst PANid */
    beacon_request[index++] = 0xFF;
    beacon_request[index++] = 0xFF;

    /* dst address */
    beacon_request[index++] = 0xFF;
    beacon_request[index++] = 0xFF;
    
    beacon_request[index++] = BEACONREQUEST;

    index += CRC_SIZE;
    
    bool masr_status = false;
    
    /* The radio transceiver will be in TRX_OFF state here and with the channel
     * set correctly.
     */
    if (MAC_SUCCESS != ieee802_15_4_set_pan_id(BROADCAST)) {
        tat_go_to_sleep();
    } else if (true != ieee802_15_4_send(index, beacon_request)) {
        tat_go_to_sleep();
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_BUSY_SCANNING);
        mac_param.scan.msc->ResultListSize = 0;
        mac_param.scan.msc->ScanType = MLME_SCAN_TYPE_ACTIVE;
        masr_status = true;
    }
    
    return masr_status;
}


static bool do_passive_scan(uint8_t ScanDuration) {
    /* Calculate scan duration in symbols. */
    mac_param.scan.scan_time = (1 << ScanDuration) + 1;
    mac_param.scan.scan_time *= aBaseSuperframeDuration;
    
    /* Store current PAN_ID as specified in the IEEE802.15.4 standard. */
    mac_param.scan.saved_pan_id = IEEE802_15_4_GET_PAN_ID();
    
    bool mpsr_status = false;
    
    /* The radio transceiver will be in TRX_OFF state here and with the channel
     * set correctly.
     */
    if (MAC_SUCCESS != ieee802_15_4_set_pan_id(BROADCAST)) {
        tat_go_to_sleep();
    } else if (true != tat_set_state(RX_AACK_ON)){
        tat_go_to_sleep();
    } else if (true != vrt_timer_start_high_pri_delay(mac_param.scan.scan_time,\
                                                      scan_time_elapsed_callback, (void *)NULL)) {
        tat_go_to_sleep();
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_BUSY_SCANNING);
        mac_param.scan.msc->ResultListSize = 0;
        mac_param.scan.msc->ScanType = MLME_SCAN_TYPE_PASSIVE;
        mpsr_status = true;
    }
    
    return mpsr_status;
}


static bool do_orphan_scan(uint8_t ScanDuration) {
    /* Try to allocate some memory to build the Orphan Notification commando on. */
    uint8_t* orphan_notification = IEEE802_15_4_GET_TX_BUFFER();
    
    /* Set the scan duration in symbols. That is how long to wait for a Coordinator
     * Realignment command.
     */
    mac_param.scan.scan_time = aResponseWaitTime;
    
    uint16_t fcf = FCF_SET_FRAMETYPE(FCF_FRAMETYPE_MAC_CMD) |
                   FCF_SET_DEST_ADDR_MODE(FCF_SHORT_ADDR)   |
                   FCF_SET_SOURCE_ADDR_MODE(FCF_LONG_ADDR);
    
    uint8_t index = 0;
    
    orphan_notification[index++] = ((fcf >> 8*0) & 0xFF);
    orphan_notification[index++] = ((fcf >> 8*1) & 0xFF);

    /* sequence number */
    orphan_notification[index++] = IEEE802_15_4_GET_DSN();;

    /* dst PANid */
    orphan_notification[index++] = 0xFF;
    orphan_notification[index++] = 0xFF;

    /* dst address */
    orphan_notification[index++] = 0xFF;
    orphan_notification[index++] = 0xFF;

    /* src PANid */
    orphan_notification[index++] = 0xFF;
    orphan_notification[index++] = 0xFF;
    
    /* src address */
    memcpy((void *)(&orphan_notification[index]), (void *)(&(IEEE802_15_4_GET_EXTENDED_ADDRESS())), sizeof(uint64_t));
    index += sizeof(uint64_t);
    
    orphan_notification[index++] = ORPHANNOTIFICATION;
    index += CRC_SIZE;
    
    bool mosr_status = false;
    
    /* The radio transceiver will be in TRX_OFF state here and with the channel
     * set correctly.
     */
    if (true != ieee802_15_4_send(index, orphan_notification)) {
        tat_go_to_sleep();
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_BUSY_SCANNING);
        mac_param.scan.msc->ResultListSize = 0;
        mac_param.scan.msc->ScanType = MLME_SCAN_TYPE_ORPHAN;
        mosr_status = true;
    }
    
    return mosr_status;
}


void ieee802_15_4_scan_process_ack(void *tx_status) {
    /* A Beacon Request command or Orphan Notification has been sent. 
     * Enable the receiver if the transmission was successful, and wait for 
     * beacons or Coordinator Realignment command to arrive. A timer will also
     * be started so that the scan can terminate after the given scan duration.
     */
    uint8_t transmission_status = *((uint8_t *)tx_status);

    /* Set up the receiver and start timer. */
    if (TRAC_SUCCESS != transmission_status) {
    } else if (true != tat_set_state(RX_AACK_ON)){
    } else if (true != vrt_timer_start_high_pri_delay(mac_param.scan.scan_time,\
                                                      scan_time_elapsed_callback, (void *)NULL)) {
    } else {
        return;
    }
    
    /* An error occured when the system was doing a transition to RX_AACK_ON mode
     * or when starting the timer. The error handling is done as if the scan terminated
     * normally.
     */
    scan_time_elapsed_callback(NULL);
}


void ieee802_15_4_scan_do_confirm(void *msc) {
    /* The radio transceiver will be in TRX_OFF when it arrives here. Put back 
     * the stored PAN ID and go to sleep.
     */
    ieee802_15_4_set_pan_id(mac_param.scan.saved_pan_id);
    
    /* Verify that the pointer is valid. */
    if (NULL == msc) { return; }
    
    /* Execute the callback. */
    mac_param.scan.mlme_scan_confirm((mlme_scan_conf_t *)msc);
    
    /* Set the internal state back to IDLE. */
    IEEE802_15_4_SET_STATE(IEEE_MAC_IDLE);
}


/*                          Callbacks                                         */


static void scan_time_elapsed_callback(void *null) {
    /* Take the radio transceiver to sleep. */
    tat_go_to_sleep();
    
    /* Check if it is possible to post event. */
    if (0 == VRT_GET_ITEMS_FREE()) {
        VRT_EVENT_MISSED();
        return;
    }
    
    mlme_scan_conf_t *msc = (mlme_scan_conf_t *)mac_param.scan.msc;
       
    if (0 == msc->ResultListSize) {
        msc->status = MAC_NO_BEACON;
    } else {
        msc->status = MAC_SUCCESS;
    }
    
    /* Post event. */
    vrt_quick_post_event(ieee802_15_4_scan_do_confirm, (void *)msc);
}
/*EOF*/
