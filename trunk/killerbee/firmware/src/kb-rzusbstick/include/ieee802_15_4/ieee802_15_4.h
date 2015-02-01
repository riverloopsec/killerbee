// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file defines the user API for the IEEE 802.15.4 MAC.
 *
 * \defgroup grIEEE802_15_4 The IEEE 802.15.4-2003 Medium Access Control Layer
 * \ingroup grModules
 *
 *      This module implements a size and performance optimized version of the
 *      Medium Access Control Layer defined in the IEEE 802.15.4 standard. This
 *      standard defines a classic request-confirm protocol for access and control
 *      of a wireless medium. The following schemes have been used to reduce code
 *      size and to increase performance:
 *       -  Requests with an immidiate return do not have an associated confirm
 *          primitive. Many of the request primitives defined in the standard is
 *          of a synchronous nature (immidiate return) and a performance gain is
 *          achieved by removing the confirm message.
 *
 *       -  This implementation can only parse one incomming frame at the time. No
 *          pipelining of frames is possible. This could give a potential problem
 *          during active scan. However, great care has been taken to speed optimize
 *          the frame parser so that the scan can still handle multiple beacons
 *          received after the transmission of the beacon request. The timeliness 
 *          of the scan path is within the LIFS and SIFS defined by the standard.
 *
 *       -  Beaconing and GTS is not supported. These are parts of the IEEE 802.15.4
 *          standard that require considerable CPU time. The adoption of these features
 *          is also limited at the moment. However, the implementation is ready to
 *          have these features added at a later date. The typical cost is a HW 
 *          timer and about 8K bytes of program memory.
 *
 *       -  Relaxed API compliance. The IEEE 802.15.4 standard defines an API that
 *          is far from optimized for 8-bit microcontrollers. Many primitives have
 *          far too long parameter lists and costly items such as 64-bit variables.
 *          Where possible, the API has been optimized at the cost of strict compliance.
 *          At the same time it should be easy for the programmer to see the
 *          resemblance with the standard.
 *
 *       -  Security is not implemented. Security will be added at a later date with
 *          HW accelerator support. Doing AES-128 in FW is a costly affair, and is
 *          better done by an accelerator
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
 * $Id: ieee802_15_4.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef IEEE802_15_4_H
#define IEEE802_15_4_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "ieee802_15_4_msg_types.h"

//! \addtogroup grIEEE802_15_4
//! @{
/*================================= MACROS           =========================*/
/** Maximum size of PHY packet */
#define aMaxPHYPacketSize           (127)
/** The requested operation was completed successfully. For a transmission
 * request, this value indicates a successful transmission.
 */
#define MAC_SUCCESS                 (0x00)
/** The beacon was lost following a synchronization request. */
#define MAC_BEACON_LOSS             (0xE0)
/** A transmission could not take place due to activity on the channel, i.e.,
 * the CSMA-CA mechanism has failed.
 */
#define MAC_CHANNEL_ACCESS_FAILURE  (0xE1)
/** The GTS request has been denied by the PAN coordinator. */
#define MAC_DENIED                  (0xE2)
/** The attempt to disable the transceiver has failed. */
#define MAC_DISABLE_TRX_FAILURE     (0xE3)
/** The received frame induces a failed security check according to the security
 * suite.
 */
#define MAC_FAILED_SECURITY_CHECK   (0xE4)
/** The frame resulting from secure processing has a length that is greater than
 * aMACMaxFrameSize.
 */
#define MAC_FRAME_TOO_LONG          (0xE5)
/** The requested GTS transmission failed because the specified GTS either did
 * not have a transmit GTS direction or was not defined.
 */
#define MAC_INVALID_GTS             (0xE6)
/** A request to purge an MSDU from the transaction queue was made using an MSDU
 * handle that was not found in the transaction table.
 */
#define MAC_INVALID_HANDLE          (0xE7)
/** A parameter in the primitive is out of the valid range. */
#define MAC_INVALID_PARAMETER       (0xE8)
/** No acknowledgment was received after aMaxFrameRetries. */
#define MAC_NO_ACK                  (0xE9)
/** A scan operation failed to find any network beacons. */
#define MAC_NO_BEACON               (0xEA)
/** No response data were available following a request. */
#define MAC_NO_DATA                 (0xEB)
/** The operation failed because a short address was not allocated. */
#define MAC_NO_SHORT_ADDRESS        (0xEC)
/** A receiver enable request was unsuccessful because it could not be completed
 * within the CAP.
 */
#define MAC_OUT_OF_CAP              (0xED)
/** A PAN identifier conflict has been detected and communicated to the PAN
 * coordinator.
 */
#define MAC_PAN_ID_CONFLICT         (0xEE)
/** A coordinator realignment command has been received. */
#define MAC_REALIGNMENT             (0xEF)
/** The transaction has expired and its information discarded. */
#define MAC_TRANSACTION_EXPIRED     (0xF0)
/** There is no capacity to store the transaction. */
#define MAC_TRANSACTION_OVERFLOW    (0xF1)
/** The transceiver was in the transmitter enabled state when the receiver was
 * requested to be enabled.
 */
#define MAC_TX_ACTIVE               (0xF2)
/** The appropriate key is not available in the ACL. */
#define MAC_UNAVAILABLE_KEY         (0xF3)
/** A SET/GET request was issued with the identifier of a PIB attribute that is
 * not supported.
 */
#define MAC_UNSUPPORTED_ATTRIBUTE   (0xF4)
/** This value is used to indicate that the sender of the data frame was not
 * found in the ACL.
 */
#define MAC_NOACLENTRYFOUND         (0x08)

/* MLME-SCAN.request type */
/** Energy scan. */
#define MLME_SCAN_TYPE_ED               (0x00)
/** Active scan. */
#define MLME_SCAN_TYPE_ACTIVE           (0x01)
/** Passive scan. */
#define MLME_SCAN_TYPE_PASSIVE          (0x02)
/** Orphan scan. */
#define MLME_SCAN_TYPE_ORPHAN           (0x03)

/* various constants */
/** Highest valid channel number */
#define MAX_CHANNEL                     (26)

/* Association status values from table 68 */
/** Association status code value. */
#define ASSOCIATION_SUCCESSFUL          (0)
/** Association status code value. */
#define PAN_AT_CAPACITY                 (1)
/** Association status code value. */
#define PAN_ACCESS_DENIED               (2) 
/** Association status code value. */
#define ASSOCIATION_RESERVED            (3)

/* these Macros are just for the API */

/** Value for the address mode, where no address is given. */
#define WPAN_ADDRMODE_NONE  (0x00)
/** Value for the address mode, where a 16 bit short address is given. */
#define WPAN_ADDRMODE_SHORT (0x02)
/** Value for the address mode, where a 64 bit long address is given. */
#define WPAN_ADDRMODE_LONG  (0x03)
/** Value for TxOptions parameter in ieee802_15_4_data_request(). */
#define WPAN_TXOPT_OFF       (0x00)
/** Value for TxOptions parameter in ieee802_15_4_data_request(). */
#define WPAN_TXOPT_ACK       (0x01)
/** Value for TxOptions parameter in ieee802_15_4_data_request(). */
#define WPAN_TXOPT_INIDIRECT (0x04)

/** Flag value for capability information field. */
#define WPAN_CAP_ALTPANCOORD   (0x01)
/** Flag value for capability information field. */
#define WPAN_CAP_FFD           (0x02)
/** Flag value for capability information field. */
#define WPAN_CAP_PWRSOURCE     (0x04)
/** Flag value for capability information field. */
#define WPAN_CAP_RXONWHENIDLE  (0x08)
/** Flag value for capability information field. */
#define WPAN_CAP_ALLOCADDRESS  (0x80)
/*================================= TYEPDEFS         =========================*/
/*! \brief Definition of MCPS_DATA.indication callback type. */
typedef void (*ieee802_15_4_data_indication_t)(mcps_data_ind_t *mcdi);
/*! \brief Definition of MLME_ASSOCIATE.indication callback type. */
typedef void (*ieee802_15_4_associate_indication_t)(mlme_associate_ind_t *mai);
/*! \brief Definition of MLME_DISASSOCIATE.indication callback type. */
typedef void (*ieee802_15_4_disassociate_indication_t)(mlme_disassociate_ind_t *mdi);
/*! \brief Definition of MLME_ORPHAN.indication callback type. */
typedef void (*ieee802_15_4_orphan_indication_t)(mlme_orphan_ind_t *moi);
/*! \brief Definition of MLME_COMM_STATUS.indication callback type. */
typedef void (*ieee802_15_4_comm_status_indication_t)(mlme_comm_status_ind_t *mcsi);
/*! \brief Definition of MLME_COMM_STATUS.indication callback type. */
typedef void (*ieee802_15_4_beacon_notify_indication_t)(mlme_beacon_notify_ind_t *mbni);
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function will initialize the IEEE 802.15.4 MAC.
 *
 *  \note This function must be executed before any of the other IEEE 802.15.4
 *        MAC API members are called.
 *
 *  \param[in] ieee_address The device's unique 64-bit address.
 *
 *  \retval true The IEEE 802.15.4 MAC was successfully initialized.
 *  \retval false Could not start the IEEE 802.15.4 MAC. None of the functions in
 *                the API should be executed.
 */
bool ieee802_15_4_init(uint64_t ieee_address);

/*! \brief This function will disable the IEEE 802.15.4 MAC and take the radio
 *         transceiver to SLEEP.
 */
void ieee802_15_4_deinit(void);

/*! \brief This function returns the function pointer assigned to the 
 *         MCPS_DATA.indication primitive.
 */
ieee802_15_4_data_indication_t ieee802_15_4_get_mcps_data_indication(void);

/*! \brief Clear the function pointer assigned to the MCPS_DATA.indication primitive. */
void ieee802_15_4_clear_mcps_data_indication(void);

/*! \brief Clear the function pointer assigned to the MCPS_DATA.indication primitive. */
void ieee802_15_4_set_mcps_data_indication(ieee802_15_4_data_indication_t indication);

/*! \brief This function returns the function pointer assigned to the 
 *         MLME_ASSOCIATE.indication primitive.
 */
ieee802_15_4_associate_indication_t ieee802_15_4_get_mlme_associate_indication(void);

/*! \brief Clear the function pointer assigned to the MLME_ASSOCIATE.indication 
 *         primitive.
 */
void ieee802_15_4_clear_mlme_associate_indication(void);

/*! \brief Clear the function pointer assigned to the MLME_ASSOCIATE.indication 
 *         primitive.
 */
void ieee802_15_4_set_mlme_associate_indication(ieee802_15_4_associate_indication_t indication);

/*! \brief This function returns the function pointer assigned to the 
 *         MLME_DISASSOCIATE.indication primitive.
 */
ieee802_15_4_disassociate_indication_t ieee802_15_4_get_mlme_disassociate_indication(void);

/*! \brief Clear the function pointer assigned to the 
 *         MLME_DISASSOCIATE.indication primitive.
 */
void ieee802_15_4_clear_mlme_disassociate_indication(void);

/*! \brief Clear the function pointer assigned to the 
 *         MLME_DISASSOCIATE.indication primitive.
 */
void ieee802_15_4_set_mlme_disassociate_indication(ieee802_15_4_disassociate_indication_t indication);

/*! \brief This function returns the function pointer assigned to the 
 *         MLME_ORPHAN.indication primitive.
 */
ieee802_15_4_orphan_indication_t ieee802_15_4_get_mlme_orphan_indication(void);

/*! \brief Clear the function pointer assigned to the 
 *         MLME_ORPHAN.indication primitive.
 */
void ieee802_15_4_clear_mlme_orphan_indication(void);

/*! \brief Clear the function pointer assigned to the MLME_ORPHAN.indication 
 *         primitive.
 */
void ieee802_15_4_set_mlme_orphan_indication(ieee802_15_4_orphan_indication_t indication);

/*! \brief This function returns the function pointer assigned to the 
 *         MLME_COMM_STATUS.indication primitive.
 */
ieee802_15_4_comm_status_indication_t ieee802_15_4_get_mlme_comm_status_indication(void);

/*! \brief Clear the function pointer assigned to the MLME_COMM_STATUS.indication primitive. */
void ieee802_15_4_clear_mlme_comm_status_indication(void);

/*! \brief Clear the function pointer assigned to the MLME_COMM_STATUS.indication primitive. */
void ieee802_15_4_set_mlme_comm_status_indication(ieee802_15_4_comm_status_indication_t indication);

/*! \brief This function returns the function pointer assigned to the 
 *         MLME_BEACOn_NOTIFY.indication primitive.
 */
ieee802_15_4_beacon_notify_indication_t ieee802_15_4_get_mlme_beacon_notify_indication(void);

/*! \brief Clear the function pointer assigned to the 
 *         MLME_BEACON_NOTIFY.indication primitive.
 */
void ieee802_15_4_clear_mlme_beacon_notify_indication(void);

/*! \brief Clear the function pointer assigned to the 
 *         MLME_BEACON_NOTIFY.indication primitive.
 */
void ieee802_15_4_set_mlme_beacon_notify_indication(ieee802_15_4_beacon_notify_indication_t indication);

/*! \brief The MCPS-DATA.request is used to send to transmit a data frame.
 *
 *  \param[in] mdr Pointer to MCPS-DATA.request message.
 *
 *  \retval true The frame was put on the air successfully.
 *  \retval false The frame could not be put on the air.
 */
bool ieee802_15_4_data_request(mcps_data_req_t *mdr);

/*! \brief The MLME-SCAN.request primitive makes a request for a node to
 *         start a scan procedure.
 *
 *  \note ED scan is not supported in this release.
 *
 *  \param msr The MLME-SCAN.request message.
 *
 *  \retval true The MLME-SCAN.request was successfully started.
 *  \retval false An error occured when starting the MLME-SCAN.request.
 */
bool ieee802_15_4_scan_request(mlme_scan_req_t* msr);

/*! \brief This primitive is used to perform a MAC reset operation.
 *
 *  \param[in] set_default_pib If this parameter is true, the PIBs will be reset
 *                             to their default values. In any other case the 
 *                             PIBs will not be altered.
 *
 *  \retval true The IEEE 802.15.4 MAC was successfully reset.
 *  \retval false The IEEE 802.15.4 MAC could not be reset. The internal state
 *                of the MAC and PHY will be unknown.
 */
bool ieee802_15_4_reset(bool set_default_pib);

/*! \brief This primitive is used to put the radio transceiver in receive mode
 *         if the internal state of the MAC is correct.
 *
 *  \retval MAC_SUCCESS The radio transceiver has been brought to receive mode.
 *  \retval MAC_INVALID_PARAMETER The transition to receive mode was unsuccessful.
 */
uint8_t ieee802_15_4_rx_enable(void);

/*! \brief This primitive is used to do a state transition from receive mode to
 *         TRX_OFF or SLEEP. The end state will depend on the mac_pib_macRxOnWhenIdle 
 *         flag.
 *
 *  \retval MAC_SUCCESS It is guaranteed per design that the TRX_OFF or SLEEP mode
 *                      is entered.
 */
uint8_t ieee802_15_4_rx_disable(void);

/*! \brief This primitive will try to start the node as a PAN coordinator on
 *         certain channel and PAN.
 *
 *  \param[in] pan_id Chosen PAN identifier for this coordinator.
 *  \param[in] channel Channel to start the PAN.
 *  \param[in] pan_coordinator True if the device will become the PAN coordinator
 *                             for a new PAN.
 *
 *  \retval MAC_INVALID_PARAMETER One of the primitive parameters are wrong, or the
 *                                internal MAC state is wrong to execute the start 
 *                                primitive.
 *  \retval MAC_SUCCESS A new PAN was successfully started.
 */
uint8_t ieee802_15_4_start_request(uint16_t pan_id, uint8_t channel, bool pan_coordinator);

/*! \brief This function implements the MLME-ASSOCIATE.request primitive.
 *
 *  \param[in] mar Pointer to MLME-ASSOCIATE.request message.
 *
 *  \retval true The associate request command frame was sent successfully.
 *  \retval false The associate request command could not be built and sent.
 */
bool ieee802_15_4_associate_request(mlme_associate_req_t *mar);

/*! \brief This function implements the MLME-ASSOCIATE.response primitive.
 *
 *  \param[in] mar Pointer to MLME-ASSOCIATE.response message.
 *
 *  \retval true The associate response message was successfully built and put
 *               in the indirect frame buffer.
 *  \retval false Associate response could not be built.
 */
bool ieee802_15_4_associate_response(mlme_associate_resp_t *mar);

/*! \brief This function implements the MLME-DISASSOCIATE.request primitive.
 *
 *  \param[in] mdr Pointer to the MLME-DISASSOCIATE.request message.
 *
 *  \retval true The disassociate indication was successfully built and sent.
 *  \retval false The disassociate indication could not be sent.
 */
bool ieee802_15_4_disassociate_request(mlme_disassociate_req_t* mdr);

/*! \brief Implements the MLME-ORPHAN.response
 *
 * The MLME-ORPHAN.response primitive allows the next higher layer of a coordinator
 * to respond to the MLME-ORPHAN.indication primitive.
 * The MLME-ORPHAN.response primitive is generated by the next higher layer and issued
 * to its MLME when it reaches a decision about whether the orphaned device indicated
 * in the MLME-ORPHAN.indication primitive is associated.
 *
 * \param mor  A pointer to the message.
 * \retval true The Coordinator Realignment Command was built and sent successfully.
 * \retval false The Coordinator Realignment Command could not be sent. Either due to wrong
 *               internal state or errors entering transmission.
 */
bool ieee802_15_4_orphan_response(mlme_orphan_resp_t *mor);

/*! \brief This primitive modifies the PAN identifier for this device.
 *
 *  \param[in] pan_id New PAN identifier.
 *
 *  \retval MAC_INVALID_PARAMETER The PAN identifier could not be written to
 *                                the radio transceiver.
 *  \retval MAC_SUCCESS PAN Identifier written.
 */
uint8_t ieee802_15_4_set_pan_id(uint16_t pan_id);

/*! \brief This primitive modifies the short address of this device.
 *
 *  \param[in] short_address New short address for the device to use.
 *
 *  \retval MAC_INVALID_PARAMETER The short address could not be written.
 *  \retval MAC_SUCCESS The short address was updated.
 */
uint8_t ieee802_15_4_set_short_address(uint16_t short_address);

/*! \brief This primitive is used to change the current operating channel.
 *
 *  \param[in] channel New operating channel.
 * 
 *  \retval MAC_INVALID_PARAMETER Channel could not be updated.
 *  \retval MAC_SUCCESS New operating channel set.
 */
uint8_t ieee802_15_4_set_channel(uint8_t channel);
//! @}
#endif
/*EOF*/
