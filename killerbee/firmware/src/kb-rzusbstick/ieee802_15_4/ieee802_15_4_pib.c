// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  The IEEE 802.15.4 standard relies on a set of variables to store
 *         key information such as; short and long address, PAN ID, channel etc.
 *
 *         These variables form the PAN Information Base (PIB). This file implements
 *         some of the access functions for these variables. Others are implemented
 *         as function like access macros for increased performance
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
 * $Id: ieee802_15_4_pib.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "tat.h"
#include "ieee802_15_4.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_pib.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/* MAC PIB variables */
bool mac_pib_macAssociationPermit;
uint64_t mac_pib_macCoordExtendedAddress;
uint16_t mac_pib_macCoordShortAddress;
uint8_t  mac_pib_macDSN;
uint16_t mac_pib_macPANId;
uint64_t mac_pib_macExtendedAddress;
bool  mac_pib_macRxOnWhenIdle;
uint16_t mac_pib_macShortAddress;
uint16_t mac_pib_macTransactionPersistenceTime;
uint8_t mac_pib_current_channel;
bool mac_pib_macAssociationPermit;
bool mac_i_am_coordinator;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

void ieee802_15_4_pib_init(void) {
    mac_pib_macAssociationPermit          = macAssociationPermit_def;
    mac_pib_macCoordExtendedAddress       = 0;
    mac_pib_macCoordShortAddress          = macCoordShortAddress_def;
    mac_pib_macDSN                        = ((mac_pib_macExtendedAddress >> 8*0) & 0xFF);
    mac_pib_macPANId                      = macPANId_def;
    mac_pib_macShortAddress               = macShortAddress_def;
    mac_pib_macRxOnWhenIdle               = macRxOnWhenIdle_def;
    mac_pib_macTransactionPersistenceTime = macTransactionPersistenceTime_def;
    mac_pib_current_channel               = 11;
    mac_pib_macAssociationPermit          = macAssociationPermit_def;
    mac_i_am_coordinator                  = false;
}


uint8_t ieee802_15_4_set_pan_id(uint16_t pan_id) {
    uint8_t set_pan_id_status = MAC_INVALID_PARAMETER;
    
    if (true != tat_wake_up()) {
    } else {
        mac_pib_macPANId = pan_id;
        tat_set_pan_id(mac_pib_macPANId);
        set_pan_id_status = MAC_SUCCESS;
    }
    
    return set_pan_id_status;
}


uint8_t ieee802_15_4_set_short_address(uint16_t short_address) {
    uint8_t set_short_address_status = MAC_INVALID_PARAMETER;
    
    if (true != tat_wake_up()) {
    } else {
        mac_pib_macShortAddress = short_address;
        tat_set_short_address(mac_pib_macShortAddress);
        set_short_address_status = MAC_SUCCESS;
    }
    
    return set_short_address_status;
}


uint8_t ieee802_15_4_set_channel(uint8_t channel) {
    uint8_t set_channel_status = MAC_INVALID_PARAMETER;
    
    if (true != tat_wake_up()) {
    } else if (true != tat_set_channel(channel)) {
    } else {
        mac_pib_current_channel = channel;
        set_channel_status = MAC_SUCCESS;
    }
    
    return set_channel_status;
}
/*EOF*/
