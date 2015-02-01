// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This header file defines the IEEE 802.15.4 message types
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
 * $Id: ieee802_15_4_msg_types.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef IEEE802_15_4_MSG_TYPES_H
#define IEEE802_15_4_MSG_TYPES_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "ieee802_15_4_conf.h"

//! \addtogroup grIEEE802_15_4
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*! \brief  Struct defining the elements of a PANDescriptor. */
typedef struct {
    /** The coordinator addressing mode corresponding to the received beacon
     *  frame.
     */
    uint8_t CoordAddrMode;
    /** The PAN identifier of the coordinator as specified in the received
     *  beacon frame.
     */
    uint16_t CoordPANId;
    /** The address of the coordinator as specified in the received beacon frame. 
     *  If CoordAddrMode is set to WPAN_ADDRMODE_SHORT, the value is interpreted 
     *  a 16 bit address.
     */
    uint64_t CoordAddress;
    /** The current logical channel occupied by the network. */
    uint8_t LogicalChannel;
    /** The superframe specification as specified in the received beacon frame.*/
    uint16_t SuperframeSpec;
    /** The LQI at which the network beacon was received. Lower values represent
     *  lower LQI (see 6.7.8 in 802.15.4-2003).
     */
    uint8_t LinkQuality;
} ieee802_15_4_pan_descriptor_t;

/** \brief This is the MCPS-DATA.confirm message structure. */
typedef struct mcps_data_conf_tag {
    /** The status of the last MSDU transmission. */
    uint8_t     status;
} mcps_data_conf_t;

/** \brief This is the MCPS-DATA.request message structure. */
typedef struct mcps_data_req_tag {
    /** The source addressing mode for this primitive and subsequent MPDU. This
    value can take one of the following values: 0 x 00 = no address (addressing
    fields omitted). 0 x 01 = reserved. 0 x 02 = 16 bit short address. 0 x 03 =
    64 bit extended address.*/
    uint8_t    SrcAddrMode;
    /** The individual device address of the entity from which the MSDU is being transferred. */
    uint64_t    SrcAddr;
    /** The destination addressing mode for this primitive and subsequent MPDU.
    This value can take one of the following values: 0 x 00 = no address
    (addressing fields omitted). 0 x 01 = reserved. 0 x 02 = 16 bit short
    address. 0 x 03 = 64 bit extended address. */
    uint8_t     DstAddrMode;
    /** The 16 bit PAN identifier of the entity to which the MSDU is being transferred. */
    uint16_t    DstPANId;
    /** The individual device address of the entity to which the MSDU is being transferred. */
    uint64_t    DstAddr;
    /** The transmission options for this MSDU. These are a bitwise OR of one or
    more of the following: 0 x 01 = acknowledged transmission. 0 x 02 = GTS
    transmission. 0 x 04 = indirect transmission. 0 x 08 = security enabled
    transmission. */
    uint8_t     TxOptions;
    /** The number of octets contained in the MSDU to be transmitted by the MAC sublayer entity. */
    uint8_t     msduLength;
    /** The set of octets forming the MSDU to be transmitted by the MAC sublayer entity. */
    uint8_t  *data;
    /** Pointer to the function that will be executed when the primitive terminates. */
    void (*mcps_data_confirm)(mcps_data_conf_t *mdc);
    /** This storage will be used when executing the MCPS_DATA.request. */
    mcps_data_conf_t mdc;
} mcps_data_req_t;

/** \brief This is the MCPS-DATA.indication message structure. */
typedef struct mcps_data_ind_tag {
    /** The source addressing mode for this primitive corresponding to the
    received MPDU. This value can take one of the following values: 0 x 00 = no
    address (addressing fields omitted). 0 x 01 = reserved. 0 x 02 = 16 bit
    short address. 0 x 03 = 64 bit extended address.*/
    uint8_t    SrcAddrMode;
    /** The individual device address of the entity from which the MSDU was received. */
    uint64_t    SrcAddr;
    /** The destination addressing mode for this primitive corresponding to the
    received MPDU. This value can take one of the following values: 0 x 00 = no
    address (addressing fields omitted). 0 x 01 = reserved. 0 x 02 = 16 bit
    short device address. 0 x 03 = 64 bit extended device address. */
    uint8_t     DstAddrMode;
    /** The 16 bit PAN identifier of the entity to which the MSDU is being transferred. */
    uint16_t    DstPANId;
    /** The individual device address of the entity to which the MSDU is being transferred.*/
    uint64_t    DstAddr;
    /** LQI value measured during reception of the MPDU. Lower values represent lower LQI (see 6.7.8). */
    uint8_t     mpduLinkQuality;
    /** The number of octets contained in the MSDU being indicated by the MAC sublayer entity. */
    uint8_t     msduLength;
    /** The set of octets forming the MSDU being indicated by the MAC sublayer entity. */
    uint8_t msdu[];
} mcps_data_ind_t;

/** \brief This is the MLME-ASSOCIATE.confirm message structure. */
typedef struct mlme_associate_conf_tag {
    /** The short device address allocated by the coordinator on successful
    association. This parameter will be equal to 0 x ffff if the association
    attempt was unsuccessful. */
    uint16_t    AssocShortAddress;
    /** The status of the association attempt. */
    uint8_t     status;
} mlme_associate_conf_t;

/** \brief This is the MLME-ASSOCIATE.request message structure. */
typedef struct mlme_associate_req_tag {
    uint8_t     LogicalChannel;
    /** The coordinator addressing mode for this primitive and subsequent MPDU.
    This value can take one of the following values: 2=16 bit short address.
    3=64 bit extended address. */
    uint8_t     CoordAddrMode;
    /** The identifier of the PAN with which to associate. */
    uint16_t    CoordPANId;
    /** The address of the coordinator with which to associate.*/
    uint64_t    CoordAddress;
    /** Specifies the operational capabilities of the associating device. */
    uint8_t     CapabilityInformation;
    /** Pointer to the function that will be executed when the primitive terminates. */
    void (*mlme_associate_confirm)(mlme_associate_conf_t *mar);
    /** Storage for the associate confirm message. */
    mlme_associate_conf_t mac;
} mlme_associate_req_t;

/** \brief This is the MLME-ASSOCIATE.indication message structure. */
typedef struct mlme_associate_ind_tag {
    /** The address of the device requesting association. */
    uint64_t    DeviceAddress;
    /** The operational capabilities of the device requesting association. */
    uint8_t     CapabilityInformation;
} mlme_associate_ind_t;

/** \brief This is the MLME-ASSOCIATE.response message structure. */
typedef struct mlme_associate_resp_tag {
    /** The address of the device requesting association. */
    uint64_t    DeviceAddress;
    /** The short device address allocated by the coordinator on successful
    association. This parameter is set to 0xffff if the association was
    unsuccessful. */
    uint16_t    AssocShortAddress;
    /** The status of the association attempt. */
    uint8_t     status;
} mlme_associate_resp_t;

/** \brief This is the MLME-DISASSOCIATE.confirm message structure. */
typedef struct mlme_disassociate_conf_tag {
    /** The status of the disassociation attempt. */
    uint8_t     status;
} mlme_disassociate_conf_t;

/** \brief This is the MLME-DISASSOCIATE.request message structure. */
typedef struct mlme_disassociate_req_tag {
    /** The address of the device to which to send the disassociation
    notification command. */
    uint64_t    DeviceAddress;
    /** The reason for the disassociation (see 7.3.1.3.2). */
    uint8_t     DisassociateReason;
    /** Pointer to the function that will be executed when the primitive terminates. */
    void (*mlme_disassociate_confirm)(mlme_disassociate_conf_t *mdc);
    /** Storage for the associated confirm message. */
    mlme_disassociate_conf_t mdc;
} mlme_disassociate_req_t;

/** \brief This is the MLME-DISASSOCIATE.indication message structure. */
typedef struct mlme_disassociate_ind_tag {
    /** The address of the device requesting disassociation. */
    uint64_t    DeviceAddress;
    /** The reason for the disassociation (see 7.3.1.3.2). */
    uint8_t     DisassociateReason;
} mlme_disassociate_ind_t;

/** \brief This is the MLME-SCAN.confirm  message structure. */
typedef struct mlme_scan_conf_tag {
    uint8_t         status;
    /** ScanType Integer 0 x 00 0 x 03 Indicates if the type of scan performed:
    0 x 00 = ED scan (FFD only). 0 x 01 = active scan (FFD only). 0 x 02 =
    passive scan. 0 x 03 = orphan scan. */
    uint8_t         ScanType;
    /** The number of elements returned in the appropriate result
    lists. This value is 0 for the result of an orphan scan. */
    uint8_t         ResultListSize;
    /** This list returns the result from the scan. */
    union{
        uint8_t ed_value;
        ieee802_15_4_pan_descriptor_t pan_desc[IEEE802_15_4_MAX_NMBR_OF_PANDESC];
    };
} mlme_scan_conf_t;

/** \brief This is the MLME-SCAN.request message structure. */
typedef struct mlme_scan_req_tag {
    /** Indicates the type of scan performed: 0 x 00 = ED scan (FFD only).
    0 x 01 = active scan (FFD only). 0 x 02 = passive scan. 0 x 03 = orphan
    scan. */
    uint8_t         ScanType;
    /** The 5 MSBs (b27, ... , b31) are reserved. The 27 LSBs (b0, b1, ... b26)
    indicate which channels are to be scanned (1 = scan, 0 = do not scan) for
    each of the 27 valid channels (see 6.1.2). */
    uint8_t        ScanChannel;
    /** A value used to calculate the length of time to spend scanning each
    channel for ED, active, and passive scans. This parameter is ignored for
    orphan scans. The time spent scanning each channel is
    [aBaseSuperframeDuration * (2n + 1)] symbols, where n is the value of the
    ScanDuration parameter. */
    uint8_t         ScanDuration;
    /** Callback that will be executed when the MLME_SCAN.request is done. */
    void (*mlme_scan_confirm)(mlme_scan_conf_t *msc);
    /** Storage for the MLME_SCAN.confirm message. */
    mlme_scan_conf_t msc;
} mlme_scan_req_t;

/** \brief This is the MLME-COMM-STATUS.indication message structure. */
typedef struct mlme_comm_status_ind_tag {
    /** The 16 bit PAN identifier of the device from which the frame was
    received or to which the frame was being sent. */
    uint16_t        PANId;
    /** The source addressing mode for this primitive. This value can take one
    of the following values: 0 = no address (addressing fields omitted).
    0 x 01 = reserved. 0 x 02 = 16 bit short address. 0 x 03 = 64 bit extended
    address. */
    uint8_t         SrcAddrMode;
    /** The source addressing mode for this primitive. This value can take one
    of the following values: 0 = no address (addressing fields omitted). 0 x 01
    = reserved. 0 x 02 = 16 bit short address. 0 x 03 = 64 bit extended address.
    */
    uint64_t        SrcAddr;
    /** The destination addressing mode for this primitive. This value can take
    one of the following values: 0 x 00 = no address (addressing fields
    omitted). 0 x 01 = reserved. 0 x 02 = 16 bit short address. 0 x 03 = 64 bit
    extended address. */
    uint8_t         DstAddrMode;
    /** The individual device address of the device for which the frame was
    intended. */
    uint64_t        DstAddr;
    /** The communications status. */
    uint8_t         status;
} mlme_comm_status_ind_t;

/** \brief This is the MLME-ORPHAN.indication message structure. */
typedef struct mlme_orphan_ind_tag {
    /** The address of the orphaned device. */
    uint64_t        OrphanAddress;
} mlme_orphan_ind_t;

/** \brief This is the MLME-ORPHAN.response message structure. */
typedef struct mlme_orphan_resp_tag {
    /** The address of the orphaned device. */
    uint64_t        OrphanAddress;
    /** The short address allocated to the orphaned device if it is associated
    with this coordinator. The special short address 0 x fffe indicates that no
    short address was allocated, and the device will use its 64 bit extended
    address in all communications. If the device was not associated with this
    coordinator, this field will contain the value 0 x ffff and be ignored on
    receipt. */
    uint16_t        ShortAddress;
    /** TRUE if the orphaned device is associated with this coordinator or
    FALSE otherwise. */
    uint8_t         AssociatedMember;
} mlme_orphan_resp_t;

/** \brief This is the MLME-BEACON-NOTIFY.indication message structure. */
typedef struct mlme_beacon_notify_ind_tag {
    /** The beacon sequence number. */
    uint8_t         BSN;
    /** The PANDescriptor for the received beacon. */
    ieee802_15_4_pan_descriptor_t PANDescriptor;
} mlme_beacon_notify_ind_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

//! @}
#endif
/*EOF*/
