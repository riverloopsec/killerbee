// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Driver for playbakc and recording of sound
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
 * $Id: audio.c 41768 2008-05-15 18:59:21Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/// file header
#include "audio.h"

#include "compiler.h"

#include "speaker.h"
#include "mic.h"
#include "sfs.h"

#include "board.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/

//! Size of audio data buffer used in \ref audio_t
#define AUDIO_BUFFER_SIZE   128

//!@{ Macros for different timer prescales
#define AUDIO_TIMER_DIV_1         1
#define AUDIO_TIMER_DIV_8         2
#define AUDIO_TIMER_DIV_64        3
#define AUDIO_TIMER_DIV_256       4
#define AUDIO_TIMER_DIV_1024      5
//!@}

//! Selected timer prescale value
#define AUDIO_TIMER_PRESCALE_VALE AUDIO_TIMER_DIV_1

//! Macro for staring audio timer
#define AUDIO_TIMER_START TCCR1B |= (AUDIO_TIMER_PRESCALE_VALE<<CS10)

//! Macro for stoping audio timer
#define AUDIO_TIMER_STOP TCCR1B &= ~(AUDIO_TIMER_PRESCALE_VALE<<CS10)

//! Maximum timer value
#define PWM_MAX 0xFF

//! Macro for posting an audio event under ISR context
#define audio_post_event_isr() {\
    if (0 == VRT_GET_ITEMS_FREE()) {\
        VRT_EVENT_MISSED();\
    } else {\
        vrt_quick_post_event(int_evt_audio, (void *)NULL);\
    }\
}

/*========================= TYPEDEFS                 =========================*/
//! Type for holding stauts and key values for the audio driver
typedef struct {
    union {
        uint8_t buffer[AUDIO_BUFFER_SIZE];
        struct {
            uint8_t buffer1[AUDIO_BUFFER_SIZE/2];
            uint8_t buffer2[AUDIO_BUFFER_SIZE/2];
        };
    };
    int buffer_ptr;
    bool buffer_1_empty;
    bool buffer_2_empty;
    bool loop_enable;
    bool playback;
    bool close_request;
    int end;
    sfs_fstream_t* Stream;
} audio_t;

/*========================= PUBLIC VARIABLES         =========================*/


/*========================= PRIVATE VARIABLES        =========================*/
//! Flag indication initialized
static bool audio_initialized = false;

//! Stauts for the audio driver
audio_t audio_status;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int audio_init(void)
{
    // De-initialize first if audio driver allready initialized
    if (audio_initialized == true) {
        audio_deinit();
    }
    
    // Power up audio filter op-amp
    AUDIO_POWER_DDR |= (1<<AUDIO_POWER_PIN);
    AUDIO_POWER_PORT &= ~(1<<AUDIO_POWER_PIN);
    
    // Initialize microphone and speaker
    speaker_init();
    mic_init();

    audio_status.Stream = NULL;
    audio_status.playback = false;
    audio_status.close_request = false;
    
    // Enable timer1 module
    PRR &= ~(1 << PRTIM1);

    // Configure timer 1
    ICR1 = PWM_MAX;
    TCNT1 = 0;
    TIFR1 =  (1<<OCF1B)|(1<<OCF1A)|(1<<TOV1); // Reset flags
    TIMSK1 = (1<<OCIE1A); // Enable interrupt
    TCCR1A = (1<<WGM11)|(0<<WGM10)|(1<<COM1A1)|(0<<COM1A0);
    TCCR1B = (1<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(0<<CS10);

    audio_initialized = true;

    return audio_initialized ? 0 : EOF;
}

void audio_deinit(void)
{
    speaker_deinit();
    mic_deinit();
    
    // Disable timer1 module
    PRR |= (1 << PRTIM1);
    
    // Power down audio filter op-amp
    AUDIO_POWER_PORT |= 1<<AUDIO_POWER_PIN;
}

void audio_event_handler(void)
{
    // Return if not initialized
    if (true != audio_initialized) {
        return;
    }

    if (audio_status.playback == true) {
        // Fill buffer1 if empty
        if (audio_status.buffer_1_empty) {
            sfs_fsize_t count = sfs_fread((void*)audio_status.buffer1, AUDIO_BUFFER_SIZE/2, audio_status.Stream);
            if (count < AUDIO_BUFFER_SIZE/2) {
                audio_status.end = count;
                if (audio_status.loop_enable) {
                    sfs_fflush(audio_status.Stream);
                    audio_status.buffer_1_empty = false;
                }
            }
            else {
                audio_status.buffer_1_empty = false;
            }
        }
    
        // Fill buffer2 if empty
        if (audio_status.buffer_2_empty) {
            sfs_fsize_t count = sfs_fread((void*)audio_status.buffer2, AUDIO_BUFFER_SIZE/2, audio_status.Stream);
            if (count < AUDIO_BUFFER_SIZE/2) {
                audio_status.end = AUDIO_BUFFER_SIZE/2 + count;
                if (audio_status.loop_enable) {
                    sfs_fflush(audio_status.Stream);
                    audio_status.buffer_2_empty = false;
                }
            }
            else {
                audio_status.buffer_2_empty = false;
            }
        }
        
        // Close file if looping not enabled
        if (audio_status.close_request == true) {
            audio_playback_stop();
            audio_status.close_request = false;   
        }
    }
}

int audio_playback_start(unsigned char* file_name, bool loop_enable)
{    
    // Return if not initialized
    if (true != audio_initialized) {
        return EOF;
    }
    
    // Stop any ongoing playbacks
    if (audio_status.playback == true) {
        audio_playback_stop();
    }
    
    // Open audio file
    if ((audio_status.Stream = sfs_fopen((const unsigned char*)file_name, SFS_OPEN_MODE_RD)) == NULL) {
        return EOF;
    }

    // Update necessary variables
    audio_status.loop_enable = loop_enable;
    audio_status.buffer_1_empty = false;
    audio_status.buffer_2_empty = true;
    audio_status.buffer_ptr = 0;
    audio_status.end = EOF;
    audio_status.playback = true;
    audio_status.close_request = false;

    // fill first audio buffer with midle value
    for (int i=0;i<AUDIO_BUFFER_SIZE/2;++i) {
        audio_status.buffer1[i] = PWM_MAX/2;
    }
    
    // Start audio timer
    AUDIO_TIMER_START;
    
    // Display tone symbol
    lcd_symbol_set(LCD_SYMBOL_TONE);
    
    // Post event to start filling first buffer
    vrt_post_event(int_evt_audio, (void *)NULL);
    
    return 0;
}

void audio_playback_stop(void)
{
    // Return if not initialized
    if (true != audio_initialized) {
        return;
    }
    
    // If playback allready stopped, just return
    if (audio_status.playback == false) {
        return;   
    }
    
    audio_status.playback = false;
    
    AUDIO_TIMER_STOP;
                
    // Clear tone symbol
    lcd_symbol_clr(LCD_SYMBOL_TONE);
    
    // Cloase audio file
    sfs_fclose(audio_status.Stream);
    audio_status.Stream = NULL;
}

int audio_record_start(unsigned char* file_name)
{
    // Return if not initialized
    if (true != audio_initialized) {
        return EOF;
    }
    
    // Not implemented
    return EOF;
}

void audio_record_stop(void)
{
    // Return if not initialized
    if (true != audio_initialized) {
        return;
    }
    
    // Not implemented
}

bool audio_playback_active(void)
{
    return audio_status.playback;
}

ISR(TIMER1_COMPA_vect)
{
    static int oversampl = 4;
    if (!oversampl--) {
        oversampl = 4;
        // Read data from buffer if buffer not empty. If reading last byte of
        // buffer set empty-flag
        if (audio_status.buffer_ptr < AUDIO_BUFFER_SIZE/2) {
            if (audio_status.buffer_1_empty == false) {
                OCR1A = audio_status.buffer[audio_status.buffer_ptr++];
                if (audio_status.buffer_ptr == AUDIO_BUFFER_SIZE/2) {
                    audio_status.buffer_1_empty = true;
                    audio_post_event_isr();
                }
                return;
            }
        } else {
            if (audio_status.buffer_2_empty == false) {
                OCR1A = audio_status.buffer[audio_status.buffer_ptr++];
                if (audio_status.buffer_ptr == AUDIO_BUFFER_SIZE) {
                    audio_status.buffer_2_empty = true;
                    audio_post_event_isr();
                    audio_status.buffer_ptr = 0;
                }
                return;
            }
        }
        
        
        // If reached end of sound file, test for loop enable
        if (audio_status.buffer_ptr == audio_status.end) {
            audio_status.end = EOF;
            if (audio_status.loop_enable) {
                if (audio_status.buffer_ptr < AUDIO_BUFFER_SIZE/2) {
                    audio_status.buffer_1_empty = true;
                    audio_post_event_isr();
                    audio_status.buffer_ptr = AUDIO_BUFFER_SIZE/2;
                }
                else {
                    audio_status.buffer_2_empty = true;
                    audio_post_event_isr();
                    audio_status.buffer_ptr = 0;
                }
            }
            else {
                audio_status.close_request = true;
                audio_post_event_isr();
                AUDIO_TIMER_STOP;
            }
        }
    }
}
//! @}
/*EOF*/
