// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  API for Over The Air communication
*
*      
*
 * \par Application note:
 *      AVR2017: RZRAVEN FW
 *
*
* \par Documentation
*
* \author
*      Atmel Corporation: http://www.atmel.com \n
*      Support email: avr@atmel.com
*
* $Id: ota.h 41659 2008-05-14 17:42:49Z hmyklebust $
*
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
*****************************************************************************/
#ifndef __OTA_H__
#define __OTA_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>

#include "rvn_ota.h"
#include "rvn_loc.h"

#include "df.h"

//! \addtogroup applCmdAVRRAVEN3290p
//! @{
/*========================= MACROS                   =========================*/
//! Macro for sending an Over The Air response
#define ota_response(data, adr, seq_nmbr, data_size) {\
    /* Send response*/\
    ota_transaction(data, adr, seq_nmbr, data_size);\
}

//! Macro for sending a simple Over The Air response
#define ota_simple_response(response, address, seq_nmbr) {\
    /* Send response*/\
    ota_simple_transaction(response, address, seq_nmbr);\
}

//! Macro for sending an Over The Air event
#define ota_event(event, address) {\
    /* Send response*/\
    ota_simple_transaction(event, address, 0x00);\
}

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief  Request to initiate firmware writing to user section of data flash
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_fw_write_initiated(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Request to load new firmware from user section of data flash
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_fw_write_image(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Firmware packet to be stored in data flash
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_fw_write_packet(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Request revision of current running firmware
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_fw_rev_get(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Current firmware writing session complete
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_fw_write_completed(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Textmessage received
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_txtmsg(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Request to read a sheared resource
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_shared_rd(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Request for the crc32 number for a firmware image
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_crc32(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Get current user configurable node name
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_getname(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Open file on data flash
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_fopen(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Close file
 *
 *              File must be opened first. The handle returned by 
 *              the fopen request must be used when closing file
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_fclose(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Read file
 *
 *              File must be opened first. The handle returned by 
 *              the fopen request must be used when reading from file
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_fread(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Write file
 *
 *              File must be opened first. The handle returned by 
 *              the fopen request must be used when writing to file
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_fwrite(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  Sign on
 *
 *              The application must respond with the current FW revision numbers
 *              using the \ref rvn_ota_rsp_sign_on_t type.
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_cmd_sign_on(rvn_loc_evt_ota_packet_t* ota_packet);

/*! \brief  OK response
 *
 *              The last OTA command was successful
 *
 *  \param[in]  ota_packet    Pointer to the received OTA packet
 *
 */
void ota_rsp_ok(rvn_loc_evt_ota_packet_t* ota_packet);


/*! \brief  ERROR response
 *
 *              The last OTA command was NOT successful.
 *
 */
void ota_rsp_error(rvn_loc_evt_ota_packet_t* ota_packet);

/*! \brief  Send one byte Over The Air
 *
 *              This function sends on byte Over The Air with the sequence number 0x00, 
 *              and dynamic memory allocation. i.e this is guarnatied to be sent
 *              over the air. 
 *
 *  \param[in]  response    A ony-byte response
 *  \param[in]  address     Address of receiver
 *  \param[in]  seq_nmbr    Sequence number (same number as provided by the command issuing the response)
 *
 */
void ota_simple_transaction(uint8_t response, uint16_t address, uint8_t seq_nmbr);

/*! \brief  Do a OTA transaction
 *
 *              Generic Over The Air transaction
 *
 *  \param[in]  data        Pointer to the data which is to be sent over the air
 *  \param[in]  adr         Address of receiver
 *  \param[in]  seq_nmbr    Sequence number
 *  \param[in]  data_size   size of the data
 *
 */
int ota_transaction(void* data, uint16_t adr, uint8_t seq_nmbr, int data_size);
//! @}
#endif // __OTA_H__
/*EOF*/
