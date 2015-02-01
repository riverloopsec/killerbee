// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file defines the API for the IEEE 802.15.4 PAN Information Base
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
 * $Id: ieee802_15_4_pib.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef IEEE802_15_4_PIB_H
#define IEEE802_15_4_PIB_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
/*================================= MACROS           =========================*/
//! \addtogroup grIEEE802_15_4
//! @{
#define IEEE802_15_4_GET_ASSOCIATION_PERMIT()                   (mac_pib_macAssociationPermit)
#define IEEE802_15_4_SET_ASSOCIATION_PERMIT(permit_association) (mac_pib_macAssociationPermit = permit_association)
#define IEEE802_15_4_GET_COORD_EXTENDED_ADDRESS()               (mac_pib_macCoordExtendedAddress)
#define IEEE802_15_4_SET_COORD_EXTENDED_ADDRESS(coord_addr)     (mac_pib_macCoordExtendedAddress = coord_addr)
#define IEEE802_15_4_GET_COORD_SHORT_ADDRESS()                  (mac_pib_macCoordShortAddress)
#define IEEE802_15_4_SET_COORD_SHORT_ADDRESS(coord_addr)        (mac_pib_macCoordShortAddress = coord_addr)
#define IEEE802_15_4_GET_DSN()                                  (mac_pib_macDSN++) // Auto increment for each access.
#define IEEE802_15_4_GET_PAN_ID()                               (mac_pib_macPANId)
#define IEEE802_15_4_GET_EXTENDED_ADDRESS()                     (mac_pib_macExtendedAddress)
#define IEEE802_15_4_SET_EXTENDED_ADDRESS(ieee_address)         (mac_pib_macExtendedAddress = ieee_address) //!< This macro is not available to the user.
#define IEEE802_15_4_GET_RX_ON_WHEN_IDLE()                      (mac_pib_macRxOnWhenIdle)
#define IEEE802_15_4_SET_RX_ON_WHEN_IDLE(rx_on)                 (mac_pib_macRxOnWhenIdle = rx_on)
#define IEEE802_15_4_GET_SHORT_ADDRESS()                        (mac_pib_macShortAddress)
#define IEEE802_15_4_GET_TRANSACTION_PERSISTENCE_TIME()         (mac_pib_macTransactionPersistenceTime)
#define IEEE802_15_4_GET_CHANNEL()                              (mac_pib_current_channel)
#define IEEE802_15_4_GET_ASSOCIATION_PERMITTED()                (mac_pib_macAssociationPermit)
#define IEEE802_15_4_SET_ASSOCIATION_PERMITTED(permitted)       (mac_pib_macAssociationPermit = permitted)

#define IEEE802_15_4_GET_I_AM_COORDINATOR()                     (mac_i_am_coordinator)
#define IEEE802_15_4_SET_I_AM_COORDINATOR(i_am_coordinator)     (mac_i_am_coordinator = i_am_coordinator)
//! @}
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/* These global variables are not to be accessed directly. Use the defined 
 * access macros above.
 */
extern bool mac_pib_macAssociationPermit;
extern uint64_t mac_pib_macCoordExtendedAddress;
extern uint16_t mac_pib_macCoordShortAddress;
extern uint8_t  mac_pib_macDSN;
extern uint16_t mac_pib_macPANId;
extern uint64_t mac_pib_macExtendedAddress;
extern bool  mac_pib_macRxOnWhenIdle;
extern uint16_t mac_pib_macShortAddress;
extern uint16_t mac_pib_macTransactionPersistenceTime;
extern uint8_t mac_pib_current_channel;
extern bool mac_pib_macAssociationPermit;

extern bool mac_i_am_coordinator;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function will set all the defined PIBs to their default values as
 *         as defined in the IEEE 802.15.4 standard.
 */
void ieee802_15_4_pib_init(void);
#endif
/*EOF*/
