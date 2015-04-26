// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  his header file is used internally by the ZIGBEE NWK layer and must
 *         not be accessed by the end user
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
 * $Id: zigbee_internal_api.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 #ifndef ZIGBEE_INTERNAL_API_H
#define ZIGBEE_INTERNAL_API_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "ieee802_15_4.h"
#include "ieee802_15_4_msg_types.h"
#include "zigbee_msg_types.h"
/*================================= MACROS           =========================*/
#define ZIGBEE_NWK_GET_STATE() (nwk_state) //!< Returns current ZigBee NWK state.
#define ZIGBEE_NWK_SET_STATE(state) (nwk_state = state) //!< Set new ZigBee NWK state.

/* NIB ACCESS MACROS. */
/*! \brief This function like macro returns the NWK layer sequence number and 
 *         increment it with one each time the macro is called.
 */
#define ZIGBEE_GET_NWK_SEQ_NMBR() (nwkSequenceNumber++)

/*! \brief This function like macro returns the max depth of any supported ZigBee network. */
#define ZIGBEE_GET_NWK_MAX_DEPTH() (nwkMaxDepth)


/*                                 NWK FCF                                    */
#define NWK_FCF_FRAMETYPE_MASK (0x03) //!< Defines the mask for the frame type.

#define NWK_FCF_FRAMETYPE_DATA (0x00) //!< Defines the Data Frame type.
#define NWK_FCF_FRAMETYPE_NWK_CMD (0x01)  //!< Defines the Network Command Frame type.

#define NWK_FCF_GET_FRAME_TYPE(x) ((x) & NWK_FCF_FRAMETYPE_MASK) //!< A macro to get the NWK frame type.
#define NWK_FCF_SET_FRAME_TYPE(x) (x) //!< A macro to set the NWK frame type.


#define NWK_FCF_PROTOCOL_VERSION_MASK (0x0F) //!< Defines the mask for the protocol version.
#define NWK_FCF_PROTOCOL_VERSION_OFFSET (0x02) //!< Defines the offset of the protcol version.

/*! \brief A macro to get the NWK protocol version. */
#define NWK_FCF_GET_PROTOCOL_VERSION(x) (((x) >> NWK_FCF_PROTOCOL_VERSION_OFFSET) & NWK_FCF_FRAMETYPE_MASK)

/*! \brief A macro to set the NWK protocol version. */
#define NWK_FCF_SET_PROTOCOL_VERSION(x) ((x) << NWK_FCF_PROTOCOL_VERSION_OFFSET)

#define NWK_FCF_DISCOVER_ROUTE_MASK (0x03) //!< Defines the mask for the discover route.
#define NWK_FCF_DISCOVER_ROUTE_OFFSET (0x05) //!< Defines the offset of the discover route.

#define NWK_SUPPRESS_ROUTE_DISCOVERY (0x00) //!< Defines the suppress route discovery type.
#define NWK_ENABLE_ROUTE_DISCOVERY (0x01) //!< Defines the enable route discovery type.
#define NWK_FORCE_ROUTE_DISCOVERY (0x02) //!< Defines the force route discovery type.
#define NWK_RESERVED_ROUTE_DISCOVERY (0x03) //!< Defines the reserved route discovery type.

/*! \brief A macro to get the discover route sub-field. */
#define NWK_FCF_GET_DISCOVER_ROUTE(x) (((x) >> NWK_FCF_PROTOCOL_VERSION_OFFSET) & NWK_FCF_DISCOVER_ROUTE_MASK)

/*! \brief A macro to set the discover route sub-field. */
#define NWK_FCF_SET_DISCOVER_ROUTE(x) ((x) << NWK_FCF_PROTOCOL_VERSION_OFFSET)

#define NWK_FCF_MULTICAST_MASK (0x01) //!< Defines the mask for the multicast sub-field.
#define NWK_FCF_MULTICAST_OFFSET (0x07) //!< Defines the offset of the multicast route sub-field.

#define NWK_FCF_UNICAST (0x00) //!< Defines the unicast type.
#define NWK_FCF_MULTICAST (0x01) //!< Defines the multicast type.

/*! \brief A macro to get the multicast sub-field. */
#define NWK_FCF_GET_MULTICAST(x) (((x) >> NWK_FCF_MULTICAST_OFFSET) & NWK_FCF_MULTICAST_MASK)

/*! \brief A macro to set the multicast sub-field. */
#define NWK_FCF_SET_MULTICAST(x) ((x) << NWK_FCF_MULTICAST_OFFSET)

#define NWK_FCF_SECURITY_MASK (0x01) //!< Defines the mask for the security sub-field.
#define NWK_FCF_SECURITY_OFFSET (0x08) //!< Defines the offset of the security route sub-field.

#define NWK_FCF_SECURITY_OFF (0x00) //!< Defines the security off type.
#define NWK_FCF_SECURITY_ON (0x01) //!< Defines the security on type.

/*! \brief A macro to get the security sub-field. */
#define NWK_FCF_GET_SECURITY(x) (((x) >> NWK_FCF_SECURITY_OFFSET) & NWK_FCF_SECURITY_MASK)

/*! \brief A macro to set the security sub-field. */
#define NWK_FCF_SET_SECURITY(x) ((x) << NWK_FCF_SECURITY_OFFSET)

#define NWK_FCF_SOURCE_ROUTE_MASK (0x01) //!< Defines the mask for the source route sub-field.
#define NWK_FCF_SOURCE_ROUTE_OFFSET (0x09) //!< Defines the offset of the source route sub-field.

#define NWK_FCF_SOURCE_ROUTE_NOT_PRESENT (0x00) //!< Defines the source route present type.
#define NWK_FCF_SOURCE_ROUTE_PRESENT (0x01) //!< Defines the source route present type.

/*! \brief A macro to get the source route sub-field. */
#define NWK_FCF_GET_SOURCE_ROUTE(x) (((x) >> NWK_FCF_SOURCE_ROUTE_OFFSET) & NWK_FCF_SOURCE_ROUTE_MASK)

/*! \brief A macro to set the source route sub-field. */
#define NWK_FCF_SET_SOURCE_ROUTE(x) ((x) << NWK_FCF_SOURCE_ROUTE_OFFSET)

/*! \brief Defines the mask for the Destination Address sub-field. */
#define NWK_FCF_DESTINATION_IEEE_ADDRESS_MASK (0x01)

/*! \brief Defines the offset of the Destination Address sub-field. */
#define NWK_FCF_DESTINATION_IEEE_ADDRESS_OFFSET (0x0A)

/*! \brief Defines the Destination IEEE Address present type. */
#define NWK_FCF_DESTINATION_IEEE_ADDRESS_NOT_PRESENT (0x00)

/*! \brief Defines the source route present type. */
#define NWK_FCF_DESTINATION_IEEE_ADDRESS_PRESENT (0x01)

/*! \brief A macro to get the Destination Address sub-field. */
#define NWK_FCF_GET_DESTINATION_IEEE_ADDRESS(x) (((x) >> NWK_FCF_DESTINATION_IEEE_ADDRESS_OFFSET) & NWK_FCF_DESTINATION_IEEE_ADDRESS_MASK)

/*! \brief A macro to set the Destination Address sub-field. */
#define NWK_FCF_SET_DESTINATION_IEEE_ADDRESS(x) ((x) << NWK_FCF_DESTINATION_IEEE_ADDRESS_OFFSET)

/*! \brief Defines the mask for the Source Address sub-field. */
#define NWK_FCF_SOURCE_IEEE_ADDRESS_MASK (0x01)

/*! \brief Defines the offset of the Source Address sub-field. */
#define NWK_FCF_SOURCE_IEEE_ADDRESS_OFFSET (0x0B)

/*! \brief Defines the Source IEEE Address present type. */
#define NWK_FCF_SOURCE_IEEE_ADDRESS_NOT_PRESENT (0x00)

/*! \brief Defines the Source IEEE Address present type. */
#define NWK_FCF_SOURCE_IEEE_ADDRESS_PRESENT (0x01)

/*! \brief A macro to get the Source Address sub-field. */
#define NWK_FCF_GET_SOURCE_IEEE_ADDRESS(x) (((x) >> NWK_FCF_SOURCE_IEEE_ADDRESS_OFFSET) & NWK_FCF_SOURCE_IEEE_ADDRESS_MASK)

/*! \brief A macro to set the Source Address sub-field. */
#define NWK_FCF_SET_SOURCE_IEEE_ADDRESS(x) ((x) << NWK_FCF_SOURCE_IEEE_ADDRESS_OFFSET)
/*================================= TYEPDEFS         =========================*/

typedef enum ZIGBEE_NWK_STATE_ENUM_TAG {
    NWK_IDLE = 0, //!< ZigBee NWK layer ready to run.
    NWK_UNINITIALIZED, //!< ZigBee NWK layer not ready to run.
    NWK_BUSY_DISCOVERING, //!< The ZigBee NWK layer is busy doing a NLME_NETWORK_DISCOVERY.request.
    NWK_FORMED, //!< Coordinator started.
    NWK_STARTED, //!< Router started.
    NWK_JOINED, //!< Device joined.
    NWK_LEAVING, //!< Node is busy leaving the network. Typically waiting for the MLME_DISASSOCIATE.request to be done.
} zigbee_nwk_state_t;

/*! \brief This union is used to store the ZigBee NWK layer parameters.  */
typedef union ZIGBEE_NWK_PARAMTERS_UNION_TAG {
    /* Variables used during Network Discovery. */
    struct {
        void (*nlme_callback_discovery_confirm)(nlme_network_discovery_conf_t* ndc);
        nlme_network_discovery_conf_t *ndc;
    } discovery;
    
    /* Variables used during Nework Join. */
    struct {
        uint16_t parent_address; //!< Address of the parent that the node is trying to join.
        void (*nlme_callback_join_confirm)(nlme_join_conf_t* njc);
        nlme_join_conf_t *njc;
    } join;
    
    /* Variables used during join indication. */
    struct {
        uint16_t allocted_address; //!< Short address just allocated.
        uint16_t capability_information; //!< Capability Information for address that just tried to associate.
    } join_ind;
    
    /* Variables used during leave. */
    struct {
        void (*nlme_callback_leave_confirm)(nlme_leave_conf_t* nlc);
        nlme_leave_conf_t *confirm;
        uint64_t node_to_leave;
    } leave;
} zigbee_nwk_parameters_t;
/*================================= GLOBAL VARIABLES =========================*/
/*! \brief Do not access directly as global variable. Use associated access 
 *         functions.
 */
extern zigbee_nwk_state_t nwk_state;


/*!\brief Storage container for parameters used internally. Must under no circumstance
 *        be accessed by the user. This will potentially keep the ZigBee NWK layer from
 *        working correct.
 */
extern zigbee_nwk_parameters_t nwk_param;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief Event handler that executes the user selected NLDE-DATA.indication
 *         callback.
 *
 *  \param[in] ndi Pointer to NLDE-DATA.indication message.
 */
void zigbee_data_indication_do_callback(void *ndi);

/*! \brief Event handler that receives the parsed NLME-JOIN.indication message
 *         and then executes the associated user callback.
 *
 *  \param[in] nji Pointer to NLME-JOIN.indication message. Is sent as a void 
 *                 pointer, so it will be casted to the correct message type.
 */
void zigbee_join_indication_do_callback(void *nji);

/*! \brief This function will be called to execute the 
 *         NLME-NETWORK-DISCOVERY.confirm event handler.
 *
 *  \param[in] ndc Pointer to the NLME-NETWROK-DISCOVERY.confirm message.
 */
void zigbee_network_discovery_confirm_do_callback(void *ndc);


/* IEEE 802.15.4 Indication callbacks. */


/*! \brief This function is the event handler for all MCPS-DATA.indication
 *         messages sent to the NWK layer.
 *
 *  \param[in] mcdi Pointer to the MCPS-DATA.indication message.
 */
void mac_data_indication_callback(mcps_data_ind_t *mcdi);

/*! \brief This function is the event handler for all MLME-ASSOCIATE.indication
 *         messages sent to the NWK layer.
 *
 *  \param[in] mai Pointer to the MLME-ASSOCIATE.indication message.
 */
void mac_associate_indication_callback(mlme_associate_ind_t *mai);

/*! \brief Callback excuted when the IEEE 802.15.4 MAC is reporting that a
 *         disassociation notification command has been received.
 *
 *  \param[in] mdi Pointer to the MLME-DISASSOCIATE.indication message.
 */
void mac_disassociate_indication_callback(mlme_disassociate_ind_t *mdi);

/*! \brief Callback excuted when the IEEE 802.15.4 MAC is reporting that a
 *         MLME-ORPHAN.indication message has been received.
 *
 *  \param[in] moi Pointer to the MLME-ORPHAN.indication message.
 */
void mac_orphan_indication_callback(mlme_orphan_ind_t *moi);

/*! \brief Callback excuted when the IEEE 802.15.4 MAC is reporting that a
 *         MLME-COMM-STATUS.indication message has been received.
 *
 *  \param[in] mcsi Pointer to the MLME-COMM-STATUS.indication message.
 */
void mac_comm_status_indication_callback(mlme_comm_status_ind_t *mcsi);
#endif
/*EOF*/
