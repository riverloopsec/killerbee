// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This header file is used internally by the IEEE 802.15.4 MAC and must
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
 * $Id: ieee802_15_4_internal_api.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef IEEE802_15_4_INTERNAL_API_H
#define IEEE802_15_4_INTERNAL_API_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_msg_types.h"
/*================================= MACROS           =========================*/
#define IEEE802_15_4_GET_STATE() (ieee802_15_4_state) //!< Returns current IEEE 802.15.4 MAC state.

/*! \brief Set new IEEE 802.15.4 MAC state.
 *
 *  \param[in] state New IEEE 802.15.4 state.
 */
#define IEEE802_15_4_SET_STATE(state) do { \
    ENTER_CRITICAL_REGION();               \
    ieee802_15_4_state = state;            \
    LEAVE_CRITICAL_REGION();               \
} while (0)

/*! \brief This function like macro returns the buffer that is used to build
 *         indications in.
 */
#define IEEE802_15_4_GET_INDICATION_BUFFER() (indication_buffer)

/*! \brief This function like macro returns the buffer that is used to build
 *         IEEE 802.15.4 MAC command frames in.
 */
#define IEEE802_15_4_GET_TX_BUFFER() (ieee802_15_4_tx_buffer)

/** MCPS-DATA.request TxOption: ACKTRANSMISSION */
#define ACK_TRANSMISSION         (0x01)
/** MCPS-DATA.request TxOption: GTSTRANSMISSION */
#define GTS_TRANSMISSION         (0x02)
/** MCPS-DATA.request TxOption: INDIRECTTRANSMISSION */
#define INDIRECT_TRANSMISSION    (0x04)
/** MCPS-DATA.request TxOption: SECURITYTRANSMISSION */
#define SECURITY_TRANSMISSION    (0x08)

/** Size of FCF in octets */
#define FCF_SIZE                (2U)

/**
 * Defines a mask for the frame type. (Table 65 IEEE 802.15.4 Specification)
 */
#define FCF_FRAMETYPE_MASK      (0x07)

/**
 * Defines the beacon frame type. (Table 65 IEEE 802.15.4 Specification)
 */
#define FCF_FRAMETYPE_BEACON    (0x00)

/**
 * Define the data frame type. (Table 65 IEEE 802.15.4 Specification)
 */
#define FCF_FRAMETYPE_DATA      (0x01)

/**
 * Define the ACK frame type. (Table 65 IEEE 802.15.4 Specification)
 */
#define FCF_FRAMETYPE_ACK       (0x02)

/**
 * Define the command frame type. (Table 65 IEEE 802.15.4 Specification)
 */
#define FCF_FRAMETYPE_MAC_CMD   (0x03)

/**
 * A macro to set the frame type.
 */
#define FCF_SET_FRAMETYPE(x)            (x)

/**
 * A macro to get the frame type.
 */
#define FCF_GET_FRAMETYPE(x)            ((x) & FCF_FRAMETYPE_MASK)

/** The mask for the security enable bit of the FCF */
#define FCF_SECURITY_ENABLED    (1U << 3)

/** The mask for the frame pending bit of the FCF */
#define FCF_FRAME_PENDING       (1U << 4)

/** The mask for the ACK request bit of the FCF */
#define FCF_ACK_REQUEST         (1U << 5)

/** The mask for the intra PAN bit of the FCF */
#define FCF_INTRA_PAN           (1U << 6)

/** Address Mode: NO ADDRESS */
#define FCF_NO_ADDR             (0x00)
/** Address Mode: RESERVED */
#define FCF_RESERVED_ADDR       (0x01)
/** Address Mode: SHORT */
#define FCF_SHORT_ADDR          (0x02)
/** Address Mode: LONG */
#define FCF_LONG_ADDR           (0x03)
/** Defines the mask for the FCF address mode */
#define FCF_ADDR_MASK           (3)

/** Defines the offset of the source address */
#define FCF_SOURCE_ADDR_OFFSET  (14)

/** Defines the offset of the destination address */
#define FCF_DEST_ADDR_OFFSET    (10)

/** Macro to set the source address mode */
#define FCF_SET_SOURCE_ADDR_MODE(x)     ((x) << FCF_SOURCE_ADDR_OFFSET)
/** Macro to get the source address mode */
#define FCF_GET_SOURCE_ADDR_MODE(x)     (((x) >> FCF_SOURCE_ADDR_OFFSET) & FCF_ADDR_MASK)
/** Macro to set the destination address mode */
#define FCF_SET_DEST_ADDR_MODE(x)       ((x) << FCF_DEST_ADDR_OFFSET)
/** Macro to get the destination address mode */
#define FCF_GET_DEST_ADDR_MODE(x)       (((x) >> FCF_DEST_ADDR_OFFSET) & FCF_ADDR_MASK)

/**
 * Defines a mask for the frame version.
 */
#define FCF_FRAMEVERSION_MASK   (0x3000)

/**
 * Defines the IEEE 802.15.4-2003 frame version.
 */
#define FCF_FRAMEVERSION_2003   (0x00)

/**
 * Defines the  IEEE 802.15.4-2006 frame version.
 */
#define FCF_FRAMEVERSION_2006   (0x01)

/**
 * A macro to set the frame version.
 */
#define FCF_SET_FRAMEVERSION(x) ((x) << 12)

/**
 * A macro to get the frame version.
 */
#define FCF_GET_FRAMEVERSION(x) (((x) & FCF_FRAMEVERSION_MASK) >> 12)

/** Battery life extention bit position */
#define BATT_LIFE_EXT_BIT_POS   (12)
/** PAN coordinator bit position */
#define PAN_COORD_BIT_POS       (14)
/** Association permit bit position */
#define ASSOC_PERMIT_BIT_POS    (15)

/** Mask of the GTS descriptor counter */
#define GTS_DESCRIPTOR_COUNTER_MASK (0x07)

/** Mask for the number of short addresses pending */
#define NUM_SHORT_PEND_ADDR(x)     ((x) & 0x07)
/** Mask for the number of long addresses pending */
#define NUM_LONG_PEND_ADDR(x)      (((x) & 0x70) >> 4)

/**
 * Defines the length of an ACK frame.
 */
#define ACK_FRAME_LEN           (0x05)

/**
 * Defines the min. length of a frame which is not an ACK
 */
#define MIN_FRAME_LEN           (0x08)

/** Size of CRC in octets */
#define CRC_SIZE                (2)

/** Generic 16 bit broadcast address */
#define BROADCAST   (0xFFFF)

/* various constants */
/** Lowest valid channel number
 */
#define MIN_CHANNEL                     (11)
/** Highest valid channel number
 */
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
/*================================= TYEPDEFS         =========================*/
typedef enum MAC_MESSAGE_ENUM_TAG {
/* MAC Command Frames (table 67) */
/** Command Frame Identifier for Association Request */
    ASSOCIATIONREQUEST          = (0x01),
/** Command Frame Identifier for Association Response */
    ASSOCIATIONRESPONSE,
/** Command Frame Identifier for Disassociation Notification */
    DISASSOCIATIONNOTIFICATION,
/** Command Frame Identifier for Data Request */
    DATAREQUEST,
/** Command Frame Identifier for PANID Conflict Notification */
    PANIDCONFLICTNOTIFICAION,
/** Command Frame Identifier for Orphan Notification */
    ORPHANNOTIFICATION,
/** Command Frame Identifier for Beacon Request */
    BEACONREQUEST,
/** Command Frame Identifier for Coordinator Realignment */
    COORDINATORREALIGNMENT,
/** Command Frame Identifier for GTS Request */
    GTSREQUEST,

/* These are not MAC command frames but listed here as they are needed
 * in the msgtype field of a frame_buffer_t struct. */

/** message is a directed orphan realignment command */
    ORPHANREALIGNMENT,
/** message is a beacon frame (in response to a beacon request cmd) */
    BEACON_MESSAGE,
/** message type field value for implicite poll without request */
    DATAREQUEST_IMPL_POLL,
/** message type field value for Null frame */
    NULL_FRAME,
/** message type field value for MCPS message */
    MCPS_MESSAGE,
} mac_message_t;


typedef enum IEEE802_15_4_STATE_ENUM_TAG {
    IEEE_MAC_IDLE = 0,
    IEEE_MAC_UNINITIALIZED,
    IEEE_MAC_BUSY_SCANNING,
    IEEE_MAC_TX_WAITING_ACK,
    IEEE_MAC_ASSOCIATED,
    IEEE_MAC_STARTED,
    IEEE_MAC_WAITING_ASSOCIATION_REQUEST_ACK,
    IEEE_MAC_WAITING_TO_SEND_DATA_REQUEST,
    IEEE_MAC_WAITING_DATA_REQUEST_ACK,
    IEEE_MAC_WAITING_ASSOCIATION_RESPONSE,
    IEEE_MAC_ASSOCIATION_REQUEST_RECEIVED,
    IEEE_MAC_WAITING_ASSOCIATE_RESPONSE_ACK,
    IEEE_MAC_WAITING_FOR_DATA_REQUEST,
    IEEE_MAC_DISASSOCIATE_WAITING_ACK,
    IEEE_MAC_WAITING_FOR_COORD_REALIGN_ACK,
    IEEE_MAC_WAITING_BEACON_ACK,
} ieee802_15_4_state_t;


/*! \brief Union that is used to store parameters between the request and the
 *         associated confirm.
 */
typedef union IEEE802_15_4_MAC_PARAMETERS_UNION_TAG {
    struct {
        union {
            uint32_t scan_time; //!< Time in symbols the scan will last.
            uint32_t scans_remaining; //!< ED scans remaining.
        };
        uint16_t saved_pan_id; //!< PAN ID of the system when the active scan was started.
        void (*mlme_scan_confirm)(mlme_scan_conf_t *msc); //!< Confirm callback pointer.
        mlme_scan_conf_t *msc; //!< Storage for the MLME_SCAN.confirm message.
    } scan;
    
    struct {
        void (*mlme_disassociate_confirm)(mlme_disassociate_conf_t *mdr);
        mlme_disassociate_conf_t *mdc; //!< Storage for the MLME_DISASSOCIATE.confirm message.
    } disassociate;
} ieee802_15_4_mac_parameters_t;


/*! \brief Meta data stored as a part of the frame received. */
typedef struct IEEE802_15_4_PACKET_METADAT_STRUCT_TAG {
    uint8_t frame_type;
    uint8_t seq_nmbr;
    uint8_t dst_addr_mode;
    uint16_t dst_pan_id;
    uint64_t dst_addr;
    uint8_t src_addr_mode;
    uint16_t src_pan_id;
    uint64_t src_addr;
    uint8_t cmd_type;
    uint8_t *msdu;
    uint8_t msdu_length;
    uint8_t lqi;
} ieee802_15_4_meta_data_t;


/*  Struct that stores the key information about the frame currently in the
 *  system.
 */
typedef struct IEEE802_15_4_FRAME_INFO_STRUCT_TAG {
    uint8_t  frame_type;
    uint16_t frame_control;
    uint8_t  seq_num;
    uint16_t dest_panid;
    uint64_t dest_address;
    uint16_t src_panid;
    uint64_t src_address;
    uint8_t  payload_length;
    uint8_t  *payload;
    uint8_t lqi;
} frame_info_t;

/*! \brief Struct definining the IEEE 802.15.4 packet. */
typedef struct IEEE802_15_4_PACKET_STRUCT_TAG {
    uint8_t length; //!< Bytes in total stored in the packet.
    uint8_t pdu[129]; //!< Packet data unit. Will consist of both header fields and body.
    ieee802_15_4_meta_data_t meta_data; //!< Saved meta data.
} ieee802_15_4_packet_t;
/*================================= GLOBAL VARIABLES =========================*/
/* These global variables are not to be accessed directly by the user. They are
 * only available internally and access through appropriate access macros.
 */
extern ieee802_15_4_state_t ieee802_15_4_state; //!< Internal state of the IEEE 802.15.4 MAC.
extern uint8_t indication_buffer[30]; //!< Buffer to build indications in.
extern uint8_t ieee802_15_4_tx_buffer[aMaxPHYPacketSize]; //!< Buffer to build IEEE 802.15.4 MAC frames.

/*! \brief Union containing storage neccessary for the requests. */
extern ieee802_15_4_mac_parameters_t mac_param;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function is the handler that is executed when an ACK has been
 *         received after the transmission of a data frame.
 *
 *  \param[in] tx_status Pointer to memory where the status of the frame 
 *                       transmission is stored.
 */
void ieee802_15_4_data_process_ack(void *tx_status);

/*! \brief This function exectutes the user MLME-DISASSOCIATE.confirm primitive.
 *
 *  \param[in] tx_status Pointer to memory where the transmission status after the
 *                       transmitting the disassociate notification is stored.
 */
void ieee802_15_4_disassociate_process_ack(void *tx_status);

/*! \brief This function is called to handle the ACK following either an Association
 *         Request or a Data Request.
 *
 *  Per design this event handler is only executed if the IEEE 802.15.4 MAC's
 *  internal state equal IEEE_MAC_WAITING_ASSOCIATION_REQUEST_ACK or 
 *  IEEE_MAC_WAITING_DATA_REQUEST_ACK.
 *
 *  \param[in] tx_status Pointer to the storage where the transmission status is
 *                       stored.
 */
void ieee802_15_4_associate_process_ack(void *tx_status);

/*! \brief This function is used to handle timeouts in the association process.
 *
 *  \param[in] null Pointer that is returned from the vrt_kernel. This pointer 
 *                  equals NULL and is not to be used!
 */
void ieee802_15_4_associate_process_timeout(void *null);

/*! \brief This function is used to build the associate confirm message and 
 *         execute the associated callback to the user.
 *
 *  \param[in] status Status of the association process.
 *  \param[in] short_addr Short address the node shall operate with. 0xFFFF
 *                        indicates an error during the association process.
 */
void issue_associate_confirm(uint8_t status, uint16_t short_addr);

/*! \brief This function will send the association response stored in the 
 *         indirect transmission buffer.
 */
void ieee802_15_4_send_association_response(void);

/*! \brief This function is the handler that is executed when an ACK has been
 *         received after the transmission of an associate response frame.
 *
 *  \param[in] tx_status Pointer to memory where the status of the frame transmission
 *                       is stored.
 */
void ieee802_15_4_associate_response_process_ack(void *tx_status);

/*! \brief This is the callback function for handling newly arrived frames.
 *
 *         This event handler is executed whenever the TAT reports that a new
 *         frame has been received. The TAT will read the frame length and post
 *         this event handler. The radio transceiver will be put in the PLL_ON
 *         state to protect the newly arrived frame from being overwritten. The
 *         frame will be uploaded from the radio transceiver's frame buffer and 
 *         parsed.
 * 
 *  \param[in] frame_length Pointer to the length of the newly received frame.
 */
void ieee802_15_4_pd_data_indication(void *frame_length);

/*! \brief This function is called when a new data frame is received from the
 *         internal frame parser.
 *
 *   \param[in] mdi_frame_info Pointer to the internal frame parser block.
 */
void ieee802_15_4_process_data_indication(void *mdi_frame_info);

/*! \brief This function calls the user selected callback handler associated with
 *         the MCPS-DATA.indication.
 *
 *  \param[in] mdi Pointer to MCPS-DATA.inidication message.
 */
void ieee802_15_4_data_indication_do_callback(void *mdi);

/*! \brief This function is executed when the IEEE 802.15.4 MAC has received a new
 *         command frame.
 *
 *  \param[in] cmd_frame_info Pointer to frame information struct for the command.
 */
void ieee802_15_4_mlme_cmd_indication(void *cmd_frame_info);

/*! \brief This function is the handler that is executed when an ACK has been
 *         received after sending a coordinator realignment command.
 *
 *  \param[in] tx_status Pointer to memory where the result of the frame 
 *                       transmission is stored.
 */
void ieee802_15_4_coord_realign_process_ack(void *tx_status);

/*! \brief This function is used to send a raw frame onto the air.
 *
 *  \param[in] length Length of the frame to send.
 *  \param[in] data Pointer to the memory where the frame to be sent is stored.
 *
 *  \retval true Frame sent successfully.
 *  \retval false Transmission state could not be reached. Frame not sent.
 */
bool ieee802_15_4_send(uint8_t length, uint8_t *data);

/*! \brief Event handler that is executed after a frame has been transmitted.
 *
 *  \param[in] tx_status Pointer to memory where the transmission status is stored.
 */
void ieee802_15_4_send_ack_event_handler(void *tx_status);

/*! \brief This function will execute the user selected callback associated with
 *         the MLME-SCAN.request.
 *
 *  \param[in] msc Pointer to MLME-SCAN.confirm message.
 */
void ieee802_15_4_scan_do_confirm(void *msc);

/*! \brief Process the ACK received after a beacon request or orphan notification
 *         frame has been transmitted.
 *
 *  \param[in] tx_status Pointer to memory where the transmission status is stored.
 */
void ieee802_15_4_scan_process_ack(void *tx_status);

/*! \brief This function is executed when a beacon frame has been detected by the 
 *         frame parser.
 *
 *  \param[in] beacon_frame_info Pointer to frame information struct for this
 *                               beacon frame.
 */
void ieee802_15_4_mlme_beacon_indication(void *beacon_frame_info);

/*! \brief This function will build and try to send a beacon frame based on the
 *         current . */
void ieee802_15_4_send_beacon(void);

/*! \brief This function is used to handle the ACK that is received when a
 *         Beacon Frame is sent.
 *
 *  The status of the transmission will not be evaluated, since it is not used by
 *  the coordinator. The coordinator will simply be taken back to RX listen mode
 *  (RX_AACK_ON) and set the IEEE 802.15.4 MAC's internal state to IEEE_MAC_STARTED.
 *
 *  \param[in] tx_status Pointer to status value indicating the successfulness of
 *                       the beacon frame transmission (OK, NO_ACK etc.).
 */
void ieee802_15_4_process_beacon_ack(void *tx_status);

/*! \brief This function is used to execute the MLME_BEACON_NOTIFY.indication.
 *
 *  \param[in] mbni Pointer to the MLME_BEACON_NOTIFY.indication message.
 *
 */
void ieee802_15_4_beacon_notify_do_callback(void *mbni);
#endif
/*EOF*/
