// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This header file defines the different ZigBee NWK layer messages
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
 * $Id: zigbee_msg_types.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
  #ifndef ZIGBEE_MSG_TYPES_H
#define ZIGBEE_MSG_TYPES_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "ieee802_15_4_msg_types.h"
#include "zigbee_conf.h"

//! \addtogroup grZigBeeNWK
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*! Definition of the Network Descriptor struct. */
typedef struct NETWORK_DESC_STRUCT_TAG {
    /** The 16-bit PAN identifier of the discovered network; the 2 highestorder
     *  bits of this parameter are reserved and shall be set to 0.
     */
    uint16_t PanID;
    /** The 64-bit PAN identifier of the network. */
    uint64_t ExtendedPanID;
    /** The current logical channel occupied by the network */
    uint8_t LogicalChannel;
    /** A ZigBee stack profile identifier indicating the stack profile in use in
     *  the discovered network.
     */
    uint8_t StackProfile;
    /** The version of the ZigBee protocol in use in the discovered network. */
    uint8_t ZigBeeVersion;
    /** This specifies how often the MAC sub-layer beacon is to be transmitted 
     *  by a given device on the network.
     */
    uint8_t BeaconOrder;
    /** For beacon-oriented networks, that is, beacon order < 15, this specifies
     *  the length of the active period of the superframe.
     */
    uint8_t SuperframeOrder;
    /** A value of TRUE indicates that at least one ZigBee router on the network 
     *  currently permits joining; That is, its NWK has been issued an 
     *  NLME-PERMIT-JOINING primitive and, the time limit if given, 
     *  has not yet expired.
     */
    bool PermitJoining;
} network_desc_t;


/*! \brief This is the NLDE-DATA.confirm message structure. */
typedef struct {
    uint8_t Status;
} nlde_data_conf_t;


/*! \brief This is the NLDE-DATA.request message structure. */
typedef struct {
    /** Destination address mode. Only supported is */
    uint8_t dst_addr_mode;
    uint16_t dst_addr;
    uint8_t radius;
    uint8_t non_member_radius;
    uint8_t discovery_route;
    uint8_t nsdu_length;
    uint8_t *nsdu;
    void (*nlde_callback_data_confirm)(nlde_data_conf_t* ndc);
    nlde_data_conf_t confirm;
    mcps_data_req_t mcps;
} nlde_data_req_t;


/*! \brief This is the NLDE-DATA.indication message structure. */
typedef struct {
    /** Integer 0x01 or 0x02 The type of destination address supplied by the 
     *  DstAddr parameter; This may have one of the following two values:
     *  0x01=16-bit multicast group address 0x02=16-bit NWK address of a device 
     *  or a 16-bit broadcast address. 
     */
    uint8_t DstAddrMode;
    /** The destination address where the NSDU is sent. */
    uint16_t DstAddr; 
    /** The individual device address from which the NSDU originated. */    
    uint16_t SrcAddr;
    /** The link quality indication delivered by the MAC on receipt of this frame 
     *  as a parameter of the MCPS-DATA.indication primitive 
     */
    uint8_t LinkQuality;
    /** The number of octets comprising the NSDU being indicated. */
    uint8_t NsduLength;
    /** The set of octets forming the NSDU being indicated by the NWK sublayer entity. */
    uint8_t Nsdu[];
} nlde_data_ind_t;


/*! \brief This is the NLME-FORMATION.confirm message structure. */
typedef struct {
    uint8_t status;
} nlme_formation_conf_t;


/*! \brief This is the NLME-FORMATION.request message structure. */
typedef struct {
    uint8_t channel;
    uint16_t pan_id;
} nlme_formation_req_t;


/*! \brief This is the NLME-NETWORK-DISCOVERY.request message structure. */
typedef struct {
    /** Gives the number of networks discovered by the search. */
    uint8_t NetworkCount;
    /** The list contains the number of elements given by the NetworkCount
     *  parameter. Table 3.9 gives a detailed account of the contents of each 
     *  item. The storage that is allocated for this array and hence the maximum
     *  allowed number of network descriptor that can be stored equals the 
     */
    network_desc_t NetworkDescriptor[ZIGBEE_MAX_NMBR_OF_NWKDESC];
    /** Any Status value returned with the MLME_SCAN.confirm primitive. */
    uint8_t Status;
} nlme_network_discovery_conf_t;


/*! \brief This is the NLME-NETWORK-DISCOVERY.request message structure. */
typedef struct {
    /** Indicates what channel to be scanned. */
    uint8_t ChannelToScan;
    /** A value used to calculate the length of time to spend scanning each channel;
     *  The time spent scanning each channel is (aBaseSuperframeDuration * (2n + 1))
     *  symbols, where n is the value of the ScanDuration parameter.
     */
    uint8_t ScanDuration;
    /** Callback that will be executed when the NLME_NETWOR_DISCOVERY.request is done. */
    void (*nlme_callback_discovery_confirm)(nlme_network_discovery_conf_t* ndc);
    /** Storage that will be used for the confirm message. */
    nlme_network_discovery_conf_t ndc;
    /** Storage that is used when the IEEE 802.15.4 MAC's MLME_SCAN.request
     *  is issued.
     */
    mlme_scan_req_t msr;
} nlme_network_discovery_req_t;


/*! \brief This is the NLME-JOIN.confirm message structure. */
typedef struct {
    /** The 16-bit short address that was allocated to this device; This parameter 
     *  will be equal to 0xFFFF if the join attempt was unsuccessful.
     */
    uint16_t ShortAddress;
    /** The PAN identifier for the network of which the device is now a member. */
    uint16_t PANId;
    /** Status INVALID_REQUEST, NOT_PERMITTED, NO_NETWORKS or any status value
     *  returned from the MLME_ASSOCIATE.confirm primitive */
    uint8_t Status;
} nlme_join_conf_t;


/*! \brief This is the NLME-JOIN.request message structure. */
typedef struct {
    /** The 16-bit PAN identifier of the network to join. */
    uint16_t PANId;
    /** The parameter is 0x00 if the device is requesting to join a network 
     *  through association. The parameter is 0x01 if the device is joining 
     *  directly or rejoining the network using the orphaning procedure. The 
     *  parameter is 0x02 if the device is joining the network using the NWK
     *  rejoining procedure.
     */
    uint8_t RejoinNetwork;
    /** The parameter is TRUE if the device is attempting to join the network in 
     *  the capacity of a ZigBee router; Otherwise, it is FALSE; The parameter 
     *  is valid in requests to join through association and ignored in requests 
     *  to join directly or to rejoin through orphaning.
     */
    bool JoinAsRouter;
    /** This parameter becomes a part of the CapabilityInformation parameter 
     *  passed to the MLME-ASSOCIATE.request primitive that is generated as the 
     *  result of a successful executing of a NWK join. The values are: 
     *  - 0x01 = Mains-powered device. 
     *  - 0x00 = other power source. 
     */
    uint8_t PowerSource;
    /** This parameter indicates whether the device can be expected to receive 
     *  packets over the air during idle portions of the CAP. The values are:
     *  - TRUE = The receiver is enabled when the device is idle.
     *  - FALSE = The receiver may be disabled when the device is idle.
     *  RxOnWhenIdle shall have a value of TRUE for ZigBee coordinators and
     *  ZigBee routers operating in a non-beaconoriented network.
     */
    bool RxOnWhenIdle;
    /** Callback that will be executed when the NLME_JOIN.request is done. */
    void (*nlme_callback_join_confirm)(nlme_join_conf_t* njc);
    /** Storage that will be used for the confirm message. */
    nlme_join_conf_t njc;
    /** Storage that is used when the IEEE 802.15.4 MAC's MLME_ASSOCIATE.request
     *  is issued.
     */
    mlme_associate_req_t mar;
} nlme_join_req_t;


/*! \brief This is the NLME-JOIN.indication message structure. */
typedef struct {
    /** The network address of an entity that has been added to the network. */
    uint16_t ShortAddress;
    /** The 64-bit IEEE address of an entity that has been added to the network. */
    uint64_t ExtendedAddress;
    /** Specifies the operational capabilities of the joining device. */
    uint8_t CapabilityInformation;
} nlme_join_ind_t;


/*! \brief This is the NLME-LEAVE.confirm message structure. */
typedef struct {
    /** The 64-bit IEEE address in the request to which this is a confirmation or
     *  null if the device requested to remove itself from the network.
     */
    uint64_t DeviceAddress;
    /** The status of the corresponding request. */
    uint8_t Status;
} nlme_leave_conf_t;


/*! \brief This is the NLME-LEAVE.request message structure. */
typedef struct {
    /** The 64-bit IEEE address of the entity to be removed from the network or 
     *  NULL if the device removes itself from the network.
     */
    uint64_t DeviceAddress;
    /** This parameter has a value of TRUE if the device being asked to leave the
     *  network is also being asked to remove its child devices, if any. Otherwise 
     *  it has a value of FALSE.
     */
    bool RemoveChildren;
    /** This parameter has a value of a TRUE if the device being asked to leave 
     *  from the current parent is requested to rejoin the network; Otherwise, 
     *  the parameter has a value of FALSE.
     */
    bool Rejoin;
    /** In the case where the DeviceAddress parameter has a non-NULL value, 
     *  indicating that another device is being asked to leave the network, this
     *  parameter has a value of TRUE if the NWK layer may reuse the address 
     *  formerly in use by the leaving device and FALSE otherwise.
     */
    bool ReuseAddress;
    /** In the case where the DeviceAddress parameter has a non-NULL value, 
     *  indicating that another device is being asked to leave the network, this
     *  parameter, if it has a value of TRUE, indicates that the leave procedure
     *  outlined here and in subclause 3.7.1.8.2 but without actually transmitting 
     *  a leave command frame.
     */
    bool Silent;
    /** Callback that will be executed when the NLME_JOIN.request is done. */
    void (*nlme_callback_leave_confirm)(nlme_leave_conf_t* nlc);
    /** Storage that will be used for the confirm message. */
    nlme_leave_conf_t nlc;
    /** Storage that is used when the IEEE 802.15.4 MAC's MLME_DISASSOCIATE.request
     *  is issued.
     */
    mlme_disassociate_req_t mdr;
} nlme_leave_req_t;

/*! \brief This is the NLME-LEAVE.indication message structure. */
typedef struct {
    /** The 64-bit IEEE address of an entity that has removed itself from the
     *  network or NULL in the case that the device issuing the primitive has
     *  been removed from the network by its parent.
     */
    uint64_t DeviceAddress;
    /** This parameter has a value of TRUE if the device being asked to disassociate 
     *  from the current parent is requested to rejoin the network; Otherwise, 
     *  this parameter has a value of FALSE.
     */
    bool Rejoin;
} nlme_leave_ind_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
//! @}
#endif
/*EOF*/
