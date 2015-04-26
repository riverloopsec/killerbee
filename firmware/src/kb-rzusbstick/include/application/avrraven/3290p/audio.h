// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Functions for generating sounds on the AVR Raven
 *
 * \par Application note:
 *      AVR2017: RZRAVEN FW
 *
 * \par Documentation
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Id: audio.h 41763 2008-05-15 18:08:50Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __AUDIO_H__
#define __AUDIO_H__
/*========================= INCLUDES                 =========================*/

#include <stdbool.h>

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/
#define AUDIO_FILE_TYPE         "*.PCM"
#define AUDIO_FILE_TYPE_SIZE    (6)
#define AUDIO_FILE_TYPE_BUFFER_SIZE    (AUDIO_FILE_TYPE_SIZE + 1)

/*========================= TYEPDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief          Initialize the audio driver
 *
 * \return                              EOF on error
 */
int audio_init(void);


/*! \brief          De-initialize the audio driver
 *
 */
void audio_deinit(void);


/*! \brief          Play a sound (.pcm) file
 *
 *                  For now, the audio file must be raw data 8 bit unsigned, mono
 *                  3906 samp/sec
 *
 * \param[in]		file_name			Name of sound file
 * \param[in]		loop_enable			Set to true to enable looping
 *
 * \return                              EOF on error
 */
int audio_playback_start(unsigned char* file_name, bool loop_enable);


/*! \brief          Stop current playback
 *
 */
void audio_playback_stop(void);


/*! \brief          Determine if a playback is active
 *
 * \retval          true    Playback active
 * \retval          false   Playback not active
 */
bool audio_playback_active(void);


/*! \brief          Start recording sound to a sound (.pcm) file
 *
 *                  For now, the audio file must be raw data 8 bit unsigned, mono
 *                  3906 samp/sec.
 *
 * \param[in]		file_name			Name of sound file
 *
 * \return                              EOF on error
 */
int audio_record_start(unsigned char* file_name);


/*! \brief          Stop current recording
 *
 */
void audio_record_stop(void);


/*! \brief          Function must be called by system to serve the audio driver
 *
 */
void audio_event_handler(void);


/*! \brief          This function must invoke the audio_event_handler()
 *
 */
void int_evt_audio(void* evt);
//! @}
#endif // __AUDIO_H__
/*EOF*/
