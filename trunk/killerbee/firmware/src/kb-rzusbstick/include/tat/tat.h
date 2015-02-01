// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Header file for the Transceiver Access Toolbox.
 *
 * \defgroup grTAT The Transceiver Access Toolbox
 * \ingroup grHardwareInterfaceLayer
 *
 * The Transceiver Access toolbox provides an interface on top of the associated
 * radio transceiver low level driver. The API is tailored to work with a 
 * communication stack as the next higher layer (IEEE 802.15.4, Wireless HART etc.)
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
 * $Id: tat.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef TAT_H
#define TAT_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "rf230.h"

//! \addtogroup grTAT
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*!< Function pointer type that is posted into the VRT_KERNEL during frame transmission and reception. */
typedef void(*tat_cb_handler_t)(void*);
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function is used to initialize the TAT.
 * 
 *         After the initialization the radio transceiver will be in TRX_OFF mode.
 *
 *  \retval true The TAT and sub systems were successfully initialized.
 *  \retval false The TAT could not be initialized. The internal state and that
 *                of the radio transceiver is unknown. No other members
 *                in this API must be called.
 */
bool tat_init(void);

/*! \brief This function will put the radio transceiver to SLEEP and
 *         disable the TAT.
 */
void tat_deinit(void);

/*! \brief This function will reset the radio transceiver's state machine to the
 *         TRX_OFF state, even if it is sleeping.
 */
void tat_reset(void);

/*! \brief Put the radio transceiver to SLEEP to save power. */
void tat_go_to_sleep(void);

/*! \brief Wake the radio transceiver from SLEEP.
 *
 *  \retval true The radio transceiver was woken and is now in the TRX_OFF state.
 *  \retval false The radio transceiver could not be taken from SLEEP.
 */
bool tat_wake_up(void);

/*! \brief This function returns the PAN ID used by the radio transceiver's frame
 *         filter.
 *
 *  \note This function has not been implemented in this release, and should not
 *        be called.
 *
 *  \returns The PAN identifier used by the radio transceiver's frame filter.
 */
uint16_t tat_get_pan_id(void);

/*! \brief This function is used to set a new PAN identifier for the frame filter.
 *
 *  \param[in] pan_id New PAN identifier to be used.
 */
void tat_set_pan_id(uint16_t pan_id);

/*! \brief This function returns the short address (16-bit) used by the radio
 *         transceiver's frame filter.
 *
 *  \note This function has not been implemented for this release, and should not
 *        be called.
 *
 *  \returns The short address used by the radio transceiver's frame filer.
 */
uint16_t tat_get_short_address(void);

/*! \brief This function is used to update the short address used by the radio
 *         transceiver's frame filter.
 *
 *  \param[in] short_address New short address to be used in the frame filter.
 */
void tat_set_short_address(uint16_t short_address);

/*! \brief This function returns the extended address (64-bit) used by the radio
 *         transceiver's frame filter.
 *
 *  \note This function has not been implemented for this release, and should not
 *        be called.
 *
 *  \returns The extended address used by the radio transceiver's frame filer.
 */
uint64_t tat_get_ieee_address(void);

/*! \brief This function is used to update the short address used by the radio
 *         transceiver's frame filter.
 *
 *  \param[in] ieee_address Pointer to extended address. A pointer to the 64-bit
 *                          variable is used in this case to ensure a more efficient
 *                          handling of this variable type.
 */
void tat_set_ieee_address(const uint8_t *ieee_address);

/*! \brief This function will enable or disable coordinator functionality in the
 *         radio transceiver.
 *
 *  \param[in] iam_pan_coordinator If this parameter is true, then the PAN coordinator
 *                                 functionality in the radio transceiver will
 *                                 be enabled. The same functionality will be
 *                                 disabled if the parameter is false.
 */
void tat_set_pan_coordinator(bool iam_pan_coordinator);

/*! \brief This function is used to fill the radio transceiver's frame buffer
 *         and send this frame.
 *
 *  \param[in] length Length in bytes of the frame to be written to the frame buffer.
 *  \param[in] frame Pointer to the memory where the frame is stored.
 *
 *  \retval true The frame was successfully sent.
 *  \retval false The frame could not be sent due to a frame length that equals
 *                zero.
 */
bool tat_send_frame(uint8_t length, const uint8_t *frame);

/*! \brief This function is used to read length bytes from the radio transceiver's
 *         frame buffer.
 *
 *  \param[in] length Number of bytes to read.
 *  \param[in] rd_buffer Pointer to allocated memory where the read bytes is to
 *                       be stored.
 *
 *  \retval true Data was read from the frame buffer successfully.
 *  \retval false The rd_buffer parameter is either NULL or the suggested number
 *                bytes to read is 0.
 */
bool tat_read_frame(uint8_t length, uint8_t *rd_buffer);

/*! \brief Get current state of the radio transceiver's internal state machine.
 *
 *  \returns State of the radio transceiver.
 */
uint8_t tat_get_state(void);

/*! \brief Do a state transition in the radio transceiver's internal state machine.
 *
 *  \retval true The state transition was successful.
 *  \retval false State transition could not be done.
 */
bool tat_set_state(uint8_t new_state);

/*! \brief Get current operating channel of the radio transceiver.
 *
 *  \returns Current operating channel.
 */
uint8_t tat_get_channel(void);

/*! \brief Update the radio transceiver's operating channel.
 *
 *  \param[in] channel New operating channel.
 *
 *  \retval true Operating channel successfully updated.
 *  \retval false The operating channel could not be updated.
 */
bool tat_set_channel(uint8_t channel);

/*! \brief This function sets the event handler that is posted when a frame has
 *          been transmitted.
 *
 *  \param[in] handler Pointer to event handler function.
 */
void tat_set_tx_callback_handler(tat_cb_handler_t handler);

/*! \brief This function returns a pointer to the function that is used as event
 *         handler for an end of transmission event.
 *
 *  \returns Pointer to the function that is used as event handler. NULL is 
 *           returned if no event handler is set.
 */
tat_cb_handler_t tat_get_tx_callback_handler(void);

/*! \brief Remove the event handler associated with end of transmission event. */
void tat_clear_tx_callback_handler(void);

/*! \brief Set the event handler that is used to handle newly received frames.
 *
 *  \param[in] handler Pointer to event handler function.
 */
void tat_set_rx_callback_handler(tat_cb_handler_t handler);

/*! \brief This function returns a pointer to the function that is used as event
 *         handler for a frame reception event.
 *
 *  \returns Pointer to the function that is used as event handler. NULL is 
 *           returned if no event handler is set.
 */
tat_cb_handler_t tat_get_rx_callback_handler(void);

/*! \brief Remove the event handler associated with the reception of a new frame. */
void tat_clear_rx_callback_handler(void);
//! @}
#endif
/* EOF */
