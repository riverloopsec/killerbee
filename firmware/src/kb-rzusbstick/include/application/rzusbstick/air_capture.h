// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the API for the AirCapture application.
 *
 * \defgroup applAirCapture The Air Capture Application
 * \ingroup applRzUsbStick
 *
 *      The AirCapture application uses the radio transceiver as a passive
 *      listening device in a network to capture frames on the air and forward 
 *      them to a connected PC over USB. The AirCapture application is also 
 *      capable of doing semi-passive operations such as channel scans, 
 *      injecting frames and jamming channels.
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
 * $Id: air_capture.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef AIR_CAPTURE_H
#define AIR_CAPTURE_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
//! \addtogroup applAirCapture
//! @{
/*================================= MACROS           =========================*/
#define AC_MIN_CHANNEL (11) //!< Lowest supported IEEE 802.15.4 channel.
#define AC_MAX_CHANNEL (26) //!< Highest supported IEEE 802.15.4 channel.

#define AC_MAX_FRAME_SIZE (1 + 127 + 1) //!< Configured for IEEE 802.15.4 frames: length + PSDU + LQI.
#define AC_ACDU_FIFO_SIZE (10) //!< Should at least be 5 ACDUs in the fifo
/*================================= TYEPDEFS         =========================*/
/*! \brief Structure that defines the AirCapture Data Unit. */
typedef struct ACDU_STRUCT_TAG {
    uint8_t ac_id; //!< Event ID indicationg that this is an AirCapture. Always equal to: EVENT_STREAM_AC_DATA. 
    uint8_t length; //!< Number of bytes stored in the ACDU, ac_id inclusive.
    uint32_t time_stamp; //!< When the frame was received. Measured in symbols: one tick = 16us.
    uint8_t rssi; //!< RSSI value for the received frame.
    uint8_t crc; //!< True indicating that the CRC for the frame is correct. False indicates the opposite.
    uint8_t frame[AC_MAX_FRAME_SIZE]; //!< The frame it self. In this implementation an IEEE 802.15.4 frame.
} acdu_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function must be called to initialize the AirCapture application.
 *
 *  \retval true The AirCapture application has been initialized and is ready for
 *               use.
 *  \retval false The AirCapture application could not be started. And no other 
 *                function in this API should be called.
 *
 *  \ingroup applAirCapture
 */
bool air_capture_init(void);

/*! \brief This function is called to terminate the AirCapture application.
 *
 *         No other functions in the API should be called after this function
 *         has been executed.
 *
 *  \ingroup air_capture 
 */
void air_capture_deinit(void);

/*! \brief This function must be called periodically to get captured data from 
 *         the RF230 device driver over the USB interface to the connected PC.
 *
 *         The task function is basically an event pump, that will ensure that
 *         ACDUs are safely transported to the PC. Also scan data can be sent.
 *
 *  \ingroup air_capture
 */
void air_capture_task(void);

/*! \brief This function will set new channel for the radio transceiver to work on.
 *
 *  \param channel New channel for the radio transceiver to operate on.
 *
 *  \retval true Channel was changed successfully.
 *  \retval false Channel could not be set.
 *
 *  \ingroup air_capture
 */
bool air_capture_set_channel(uint8_t channel);

/*! \brief This function will try to start capturing frames from the air.
 *
 *  After this function has been called it is important to call the air_capture_task
 *  function periodically to prevent losing captured frames due to buffer overflow
 *  in the event pump.
 *
 *  \retval true Capture started successfully.
 *  \retval false Not possible to start capturing.
 *
 *  \ingroup air_capture
 */
bool air_capture_open_stream(void);

/*! \brief This function stops the radio transceiver from capturing frames.
 *
 *  \retval true Capture stopped successfully.
 *  \retval false Not possible to stop capturing.
 *
 *  \ingroup air_capture
 */
bool air_capture_close_stream(void);

/*! \brief This function is used to start a scan on the current channel.
 *
 *  After this function has been called it is important to call the air_capture_task
 *  function periodically to prevent loosing scan data. The tasking will ensure 
 *  that the event pump transports data from the storage buffers and to the
 *  PC.
 *
 *  \param scan_type Type of scan to be performed.
 *  \param scan_duration How long to run the chosen channel scan.
 *
 *  \retval true Scan started successfully.
 *  \retval false Scan could not be started.
 *
 *  \ingroup air_capture
 */
bool air_capture_start_channel_scan(uint8_t scan_type, uint8_t scan_duration);

/*! \brief This function will stop the scan even if it is not yet finished.
 *
 *  \retval true Scan stopped.
 *  \retval false scan could not be stopped.
 *
 *  \ingroup air_capture
 */
bool air_capture_stop_channel_scan(void);

/*! \brief This function will inject an user defined frame onto the current channel.
 *
 *  \param length Length of frame to inject.
 *  \param frame Pointer to the frame to be sent.
 *
 *  \retval true Frame injected successfully.
 *  \retval false Frame could not be injected.
 *
 *  \ingroup air_capture
 */
//bool air_capture_inject_frame(uint8_t length, uint8_t *frame);
uint8_t air_capture_inject_frame(uint8_t length, uint8_t *frame);

/*! \brief This function starts the jammer.
 *
 *  The jammer will start sending random data on the air without using CSMA 
 *  algorithm for channel access, and this way prevent other devices from 
 *  transmitting. This functionality is mainly used for test purposes. The duty
 *  cycle of the transmission is 100%, so any device within range shall evaluate
 *  the channel to be busy, and henche not send any frames.
 *
 *  \retval true Jammer started.
 *  \retval false Jammer could not be started.
 *
 *  \ingroup air_capture
 */
bool air_capture_jammer_on(void);

/*! \brief This function stops the jammer.
 *
 *  \retval true Jammer stopped successfully.
 *  \retval false Jammer could not be stopped.
 *
 *  \ingroup air_capture
 */
bool air_capture_jammer_off(void);

//! @}
#endif
/*EOF*/
