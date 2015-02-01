// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This header file contains all IEEE 802.15.4 constants and attribute
 *         identifiers
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
 * $Id: ieee802_15_4_const.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef IEEE802_15_4_CONST_H
#define IEEE802_15_4_CONST_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

//! \addtogroup grIEEE802_15_4
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
/** Maximum size of PHY packet */
#define aMaxPHYPacketSize           (127)
/** Maximum turnaround Time of the radio to switch from Rx to Tx or Tx to Rx */
#define aTurnaroundTime             (12)

/* 7.4.1 MAC Layer Constants */

/**
 * The number of symbols forming a superframe slot
 * when the superframe order is equal to 0.
 */
#define aBaseSlotDuration              (60)

/**
 * The number of symbols forming a superframe when
 * the superframe order is equal to 0.
 */
#define aBaseSuperframeDuration        (aBaseSlotDuration * aNumSuperframeSlots)

/**
 * The maximum value of the backoff exponent in the
 * CSMA-CA algorithm.
 */
#define aMaxBE                         (5)

/**
 * The maximum number of octets added by the MAC
 * sublayer to the payload of its beacon frame.
 */
#define aMaxBeaconOverhead             (75)

/**
 * The maximum size, in octets, of a beacon payload.
 */
#define aMaxBeaconPayloadLength        (aMaxPHYPacketSize - aMaxBeaconOverhead)

/**
 * The number of superframes in which a GTS descriptor
 * exists in the beacon frame of a PAN coordinator.
 */
#define aGTSDescPersistenceTime        (4)

/**
 * The maximum number of octets added by the MAC
 * sublayer to its payload without security. If security is
 * required on a frame, its secure processing may inflate
 * the frame length so that it is greater than this value. In
 * this case, an error is generated through the appropriate
 * .confirm or MLME-COMM-STATUS.indication
 * primitives.
 */
#define aMaxFrameOverhead              (25)

/**
 * The mainimum number of octets added by the MAC
 * sublayer to its payload without security.
 */
#define aMinFrameOverhead              (9)

/**
 * The maximum number of CAP symbols in a beaconenabled
 * PAN, or symbols in a nonbeacon-enabled
 * PAN, to wait for a frame intended as a response to a
 * data request frame.
 */
#define aMaxFrameResponseTime          (1220)

/**
 * The maximum number of retries allowed after a
 * transmission failure.
 */
#define aMaxFrameRetries               (3)

/**
 * The number of consecutive lost beacons that will
 * cause the MAC sublayer of a receiving device to
 * declare a loss of synchronization.
 */
#define aMaxLostBeacons                (4)

/**
 * The maximum number of octets that can be
 * transmitted in the MAC frame payload field.
 */
#define aMaxMACFrameSize               (aMaxPHYPacketSize - aMaxFrameOverhead)

/**
 * The maximum size of an MPDU, in octets, that can be
 * followed by a short interframe spacing (SIFS)
 * period.
 */
#define aMaxSIFSFrameSize              (18)

/**
 * The minimum number of symbols forming the CAP.
 * This ensures that MAC commands can still be
 * transferred to devices when GTSs are being used.
 * An exception to this minimum shall be allowed for the
 * accommodation of the temporary increase in the
 * beacon frame length needed to perform GTS
 * maintenance (see 7.2.2.1.3).
 */
#define aMinCAPLength                  (440)

/**
 * The minimum number of symbols forming a long
 * interframe spacing (LIFS) period.
 */
#define aMinLIFSPeriod                 (40)

/**
 * The minimum number of symbols forming a short
 * interframe spacing (SIFS) period.
 */
#define aMinSIFSPeriod                 (12)

/** The number of slots contained in any superframe. */
#define aNumSuperframeSlots            (16)

/**
 * The maximum number of symbols a device shall wait
 * for a response command to be available following a
 * request command.
 */
#define aResponseWaitTime              (32 * aBaseSuperframeDuration)

/**
 * The number of symbols forming the basic time period
 * used by the CSMA-CA algorithm.
 */
#define aUnitBackoffPeriod             (20)

/* PHY PIB Attributes */

/* standard pib attributes */
/** The RF channel to use for all following transmissions and receptions. */
#define phyCurrentChannel           (0x00)
/** The 5 most significant bits (MSBs) (b27,... , b31) of phyChannelsSupported
 * shall be reserved and set to 0, and the 27 LSBs (b0, b1, ... b26) shall
 * indicate the status (1=available, 0=unavailable) for each of the 27 valid
 * channels (bk shall indicate the status of channel k). */
#define phyChannelsSupported        (0x01)
/** The 2 MSBs represent the tolerance on the transmit power: 00 =  1 dB
 * 01 =  3 dB 10 =  6 dB The 6 LSBs represent a signed integer in
 * twos-complement format, corresponding to the nominal transmit power of the
 * device in decibels relative to 1 mW. The lowest value of phyTransmitPower
 * shall be interpreted as less than or equal to 32 dBm. */
#define phyTransmitPower            (0x02)
/** The CCA mode
 *  - CCA Mode 1: Energy above threshold. CCA shall report a busy medium
 * upon detecting any energy above the ED threshold.
 *  - CCA Mode 2: Carrier sense only. CCA shall report a busy medium only upon
 * the detection of a signal with the modulation and spreading characteristics
 * of IEEE 802.15.4. This signal may be above or below the ED threshold.
 *  - CCA Mode 3: Carrier sense with energy above threshold. CCA shall report a
 * busy medium only upon the detection of a signal with the modulation and
 * spreading characteristics of IEEE 802.15.4 with energy above the ED
 * threshold. */
#define phyCCAMode                  (0x03)

/* non-standard pib attributes (support for hardware-assisted MAC functions) */
/** Boolean, perform CRC checking */
#define phyCheckCRC                 (0x04)
/** Boolean, perform address recognition */
#define phyCheckAddr                (0x05)
/** Boolean, device is PAN coordinator */
#define phyPANcoord                 (0x06)
/** Boolean, perform automatic ACK sending */
#define phyAutoACK                  (0x07)
/** 64bit, hardware MAC address */
#define phyIEEEaddr                 (0x08)
/** 16bit, short MAC address */
#define phyShortAddr                (0x09)
/** 16bit, PAN identifier */
#define phyPANid                    (0x0a)
/** Boolean, perform automatic CSMA-CA and retries. */
#define phyAutoCSMACA               (0x0b)
/** RSSI of the last frame received */
#define phyRSSI                     (0x0c)
/** Max. number of frame retries for missing ACK. */
#define phyMaxFrameRetries          (0x0d)
/** Max. number of CSMA Backoffs. If 7 then 0 reties and no CSMA_CA */
#define phyMaxCSMABackoffs          (0x0e)
/** Min. BE number */
#define phyMinBE                    (0x0f)
/** Generate CRC in PD-DATA.request */
#define phyGenerateCRC              (0x10)
/** Set the pending bit in auto-generated ACKs. */
#define phySetPending               (0x11)
/** r/o attribute specifying the time the radio requires to wake up from sleep
 * mode. */
#define phyWakeupTime               (0x12)
/** Current sleep state of radio. */
#define phySleeping                 (0x13)

/** timeout for CCA in symbol periods */
#define CCA_TIMEOUT                 (8)
/** timeout for ED in symbol periods */
#define ED_TIMEOUT                  (8)


/* 7.4.2 MAC PIB Attributes */


/**
 * The maximum number of symbols
 * to wait for an acknowledgment
 * frame to arrive following a
 * transmitted data frame. This value
 * is dependent on the currently
 * selected logical channel. For 0 <=
 * phyCurrentChannel <= 10, this
 * value is equal to 120. For 11 <=
 * phyCurrentChannel <= 26, this
 * value is equal to 54.
 *
 * - @em Type: Integer
 * - @em Range: 54 or 120
 * - @em Default: 54
 */
#define macAckWaitDuration             (0x40)


/**
 * Indication of whether a coordinator
 * is currently allowing association.
 * A value of true indicates
 * that association is permitted.
 *
 * - @em Type: Boolean
 * - @em Range: true or false
 * - @em Default: false
 */
#define macAssociationPermit           (0x41)
/** default value for MIB macAssociationPermit*/
#define macAssociationPermit_def       (false)

/**
 * Indication of whether a device
 * automatically sends a data request
 * command if its address is listed in
 * the beacon frame. A value of
 * true indicates that the data
 * request command is automatically
 * sent.
 *
 * - @em Type: Boolean
 * - @em Range: true or false
 * - @em Default: true
 */
#define macAutoRequest                 (0x42)
/** default value for MIB macAutoRequest*/
#define macAutoRequest_def             (true)

/**
 * Indication of whether battery life
 * extension, by reduction of coordinator
 * receiver operation time during
 * the CAP, is enabled. A value of
 * true indicates that it is enabled.
 *
 * - @em Type: Boolean
 * - @em Range: true or false
 * - @em Default: false
 */
#define macBattLifeExt                 (0x43)
/** default value for MIB macBattLifeExt*/
#define macBattLifeExt_def             (false)

/**
 * The number of backoff periods
 * during which the receiver is
 * enabled following a beacon in
 * battery life extension mode. This
 * value is dependent on the currently
 * selected logical channel. For 0 <=
 * phyCurrentChannel <= 10, this
 * value is equal to 8. For 11 <=
 * phyCurrentChannel <= 26, this
 * value is equal to 6.
 *
 * - @em Type: Integer
 * - @em Range: 6 or 8
 * - @em Default: 6
 */
#define macBattLifeExtPeriods          (0x44)
/** default value for MIB macBattLifeExtPeriods*/
#define macBattLifeExtPeriods_def      (6)

/**
 * The contents of the beacon
 * payload.
 *
 * - @em Type: Set of octets
 * - @em Range: --
 * - @em Default: NULL
 */
#define macBeaconPayload               (0x45)

/**
 * The length, in octets, of the beacon
 * payload.
 *
 * - @em Type: Integer
 * - @em Range: 0 - aMaxBeaconPayloadLength
 * - @em Default: 0
 */
#define macBeaconPayloadLength         (0x46)
/** default value for MIB macBeaconPayloadLength*/
#define macBeaconPayloadLength_def     (0)

/**
 * Specification of how often the
 * coordinator transmits a beacon.
 * The macBeaconOrder, BO, and the
 * beacon interval, BI, are related as
 * follows: for 0 <= BO <= 14, BI =
 * aBaseSuperframeDuration * 2^BO
 * symbols. If BO = 15, the coordinator
 * will not transmit a beacon.
 *
 * - @em Type: Integer
 * - @em Range: 0-15
 * - @em Default: 15
 */
#define macBeaconOrder                 (0x47)
/** default value for MIB macBeaconOrder*/
#define macBeaconOrder_def             (15)

/**
 * The time that the device
 * transmitted its last beacon frame,
 * in symbol periods. The
 * measurement shall be taken at the
 * same symbol boundary within
 * every transmitted beacon frame,
 * the location of which is
 * implementation specific.
 * The precision of this value shall be
 * a minimum of 20 bits, with the
 * lowest four bits being the least
 * significant.
 *
 * - @em Type: Integer
 * - @em Range: 0x000000-0xffffff
 * - @em Default: 0x000000
 */
#define macBeaconTxTime                (0x48)
/** default value for MIB macBeaconTxTime*/
#define macBeaconTxTime_def            (0x000000)

/**
 * The sequence number added to the
 * transmitted beacon frame.
 *
 * - @em Type: Integer
 * - @em Range: 0x00-0xff
 * - @em Default: Random value from within the range.
 */
#define macBSN                         (0x49)

/**
 * The 64 bit address of the coordinator
 * with which the device is
 * associated.
 *
 * - @em Type: IEEE address
 * - @em Range: An extended 64bit IEEE address
 * - @em Default: -
 */
#define macCoordExtendedAddress        (0x4a)

/**
 * The 16 bit short address assigned
 * to the coordinator with which the
 * device is associated. A value of
 * 0xfffe indicates that the
 * coordinator is only using its 64 bit
 * extended address. A value of
 * 0xffff indicates that this value is
 * unknown.
 *
 * - @em Type: Integer
 * - @em Range: 0x0000-0xffff
 * - @em Default: 0xffff
 */
#define macCoordShortAddress           (0x4b)
/** default value for MIB macCoordShortAddress*/
#define macCoordShortAddress_def       (0xffff)

/**
 * The sequence number added to the
 * transmitted data or MAC command
 * frame.
 *
 * - @em Type: Integer
 * - @em Range: 0x00-0xff
 * - @em Default: Random value from within the range.
 */
#define macDSN                         (0x4c)

/**
 * true if the PAN coordinator is to
 * accept GTS requests. false otherwise.
 *
 * - @em Type: Boolean
 * - @em Range: true or false
 * - @em Default: true
 */
#define macGTSPermit                   (0x4d)
/** default value for MIB macGTSPermit*/
#define macGTSPermit_def               (true)

/**
 * The maximum number of backoffs
 * the CSMA-CA algorithm will
 * attempt before declaring a channel
 * access failure.
 *
 * - @em Type: Integer
 * - @em Range: 0-5
 * - @em Default: 4
 */
#define macMaxCSMABackoffs             (0x4e)
/** default value for MIB macMaxCSMABackoffs*/
#define macMaxCSMABackoffs_def         (4)

/**
 * The minimum value of the backoff
 * exponent in the CSMA-CA
 * algorithm. Note that if this value is
 * set to 0, collision avoidance is
 * disabled during the first iteration
 * of the algorithm. Also note that for
 * the slotted version of the CSMACA
 * algorithm with the battery life
 * extension enabled, the minimum
 * value of the backoff exponent will
 * be the lesser of 2 and the value of
 * macMinBE.
 *
 * - @em Type: Integer
 * - @em Range: 0-3
 * - @em Default: 3
 */
#define macMinBE                       (0x4f)
/** default value for MIB macMinBE*/
#define macMinBE_def                   (3)

/**
 * The 16 bit identifier of the PAN on
 * which the device is operating. If
 * this value is 0xffff, the device is
 * not associated.
 *
 * - @em Type: Integer
 * - @em Range: 0x0000-0xffff
 * - @em Default: 0xffff
 */
#define macPANId                       (0x50)
/** default value for MIB macPANId*/
#define macPANId_def                   (0xffff)

/**
 * This indicates whether the MAC
 * sublayer is in a promiscuous
 * (receive all) mode. A value of
 * true indicates that the MAC
 * sublayer accepts all frames
 * received from the PHY.
 *
 * - @em Type: Boolean
 * - @em Range: true or false
 * - @em Default: false
 */
#define macPromiscuousMode             (0x51)
/** default value for MIB macPromiscuousMode*/
#define macPromiscuousMode_def         (false)

/**
 * This indicates whether the MAC
 * sublayer is to enable its receiver
 * during idle periods.
 *
 * - @em Type: Boolean
 * - @em Range: true or false
 * - @em Default: false
 */
#define macRxOnWhenIdle                (0x52)
/** default value for MIB macRxOnWhenIdle*/
#define macRxOnWhenIdle_def            (false)

/**
 * The 16 bit address that the device
 * uses to communicate in the PAN.
 * If the device is a PAN coordinator,
 * this value shall be chosen before a
 * PAN is started. Otherwise, the
 * address is allocated by a
 * coordinator during association.
 * A value of 0xfffe indicates that the
 * device has associated but has not
 * been allocated an address. A value
 * of 0xffff indicates that the device
 * does not have a short address.
 *
 * - @em Type: Integer
 * - @em Range: 0x0000-0xffff
 * - @em Default: 0xffff
 */
#define macShortAddress                (0x53)
/** default value for MIB macShortAddress*/
#define macShortAddress_def            (0xffff)

/**
 * This specifies the length of the
 * active portion of the superframe,
 * including the beacon frame. The
 * macSuperframeOrder, SO, and the
 * superframe duration, SD, are
 * related as follows: for 0 <= SO <= BO
 * <= 14, SD =
 * aBaseSuperframeDuration * 2SO
 * symbols. If SO = 15, the
 * superframe will not be active
 * following the beacon.
 *
 * - @em Type: Integer
 * - @em Range: 0-15
 * - @em Default: 15
 */
#define macSuperframeOrder             (0x54)
/** default value for MIB macSuperframeOrder*/
#define macSuperframeOrder_def         (15)

/**
 * The maximum time (in superframe
 * periods) that a transaction is stored
 * by a coordinator and indicated in
 * its beacon.
 *
 * - @em Type: Integer
 * - @em Range: 0x0000-0xffff
 * - @em Default: 0x01f4
 */
#define macTransactionPersistenceTime  (0x55)
/** default value for MIB macTransactionPersistenceTime*/
#define macTransactionPersistenceTime_def (0x01f4)


/* 6.2.3 PHY Enumeration Definitions */
/** The CCA attempt has detected a busy channel.*/
#define PHY_BUSY                    (0x00)
/** The transceiver is asked to change its state while receiving. */
#define PHY_BUSY_RX                 (0x01)
/** The transceiver is asked to change its state while transmitting. */
#define PHY_BUSY_TX                 (0x02)
/** The transceiver is to be switched off. */
#define PHY_FORCE_TRX_OFF           (0x03)
/** The CCA attempt has detected an idle channel. */
#define PHY_IDLE                    (0x04)
/** A SET/GET request was issued with a parameter in the primitive that is out
 * of the valid range. */
#define PHY_INVALID_PARAMETER       (0x05)
/** The transceiver is in or is to be configured into the receiver enabled
 * state. */
#define PHY_RX_ON                   (0x06)
/** A SET/GET, an ED operation, or a transceiver state change was successful. */
#define PHY_SUCCESS                 (0x07)
/** The transceiver is in or is to be configured into the transceiver disabled
 * state. */
#define PHY_TRX_OFF                 (0x08)
/** The transceiver is in or is to be configured into the transmitter enabled
 * state. */
#define PHY_TX_ON                   (0x09)
/** A SET/GET request was issued with the identifier of an attribute that is not
 * supported. */
#define PHY_UNSUPPORTED_ATTRIBUTE   (0x0a)

/* non-standard values / extensions */
/** PHY_SUCCESS in phyAutoCSMACA when received ACK frame had the pending bit set
*/
#define PHY_SUCCESS_DATA_PENDING    (0x10)


/*=== MAC Status Enumeration Definitions ===*/

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
/** First part of the request was successful. Confirm will be returned shortly. */
#define MAC_WAITING_FOR_CONFIRM  (0xF5)
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
/** Highest valid channel number. */
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

//! @}
#endif
/*EOF*/
