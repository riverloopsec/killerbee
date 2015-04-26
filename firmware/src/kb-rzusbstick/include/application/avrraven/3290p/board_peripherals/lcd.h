// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven LCD driver
 *
 * \defgroup grLCDLowLevel AVRRAVEN LCD Low Level Driver
 * \ingroup grLowLevelDrivers
 *
 * Low level driver for the AVRRAVEN LCD.
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
 * $Id: lcd.h 41580 2008-05-13 20:35:51Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __LCD_H__
#define __LCD_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "bcd.h"
#include "lcd_conf.h"

#include "asy_tmr.h"

//! \addtogroup grLCDLowLevel
//! @{
/*========================= MACROS                   =========================*/
#define LCD_CLCK_FRQ            (32768)

#define LCD_DATA_BUFFER_SIZE    (65)

#define LCD_SYMBOL_FISRT        (0)
#define LCD_SYMBOL_COUNT        (34)

#define LCD_ALPHA_DIGIT_COUNT   (7)
#define LCD_NUM_DIGIT_COUNT     (4)

#define LCD_CURSOR_START_ON     false
#define LCD_CURSOR_START_OFF    true

// Duplicate usage of segments
#define LCD_SYMBOL_ARR_UP       LCD_SYMBOL_ENV_OP
#define LCD_SYMBOL_ARR_DN       LCD_SYMBOL_ENV_CL

// Symbol group defines

// Battery
#define LCD_SYMBOL_BAT_ALL      4

// Antenna
#define LCD_SYMBOL_ANTENNA_SIG_0  0
#define LCD_SYMBOL_ANTENNA_SIG_1  1
#define LCD_SYMBOL_ANTENNA_SIG_2  2
#define LCD_SYMBOL_ANTENNA_SIG_3  3
#define LCD_SYMBOL_ANTENNA_DIS    4

// Macros for fast symbol update
#define LCD_SYMBOL_SET(symbol) *(lcd_data + (symbol & 0x1F)) = *(lcd_data + (symbol & 0x1F)) | ( 1 << (symbol >> 5))
#define LCD_SYMBOL_CLR(symbol) *(lcd_data + (symbol & 0x1F)) = *(lcd_data + (symbol & 0x1F))  & ~( 1 << (symbol >> 5))

// Symbol group macros

// Battery
#define lcd_symbol_battery_remove()     lcd_symbol_clr_group(LCD_SYMBOL_INDEX_BAT_CONT, 4)
#define lcd_symbol_battery_empty()      lcd_symbol_clr_group(LCD_SYMBOL_INDEX_BAT_CAP1, 3)
#define lcd_symbol_battery_set(nmb)     lcd_symbol_battery_empty(); lcd_symbol_set_group(LCD_SYMBOL_INDEX_BAT_CAP1, nmb+1)
#define lcd_symbol_battery_init()       lcd_symbol_battery_remove(); lcd_symbol_set(LCD_SYMBOL_BAT_CONT)

// Antenna
#define lcd_symbol_antenna_clr_sig()    lcd_symbol_clr_group(LCD_SYMBOL_INDEX_ANT_SIG1, 4)
#define lcd_symbol_antenna_signal(nmb)  lcd_symbol_antenna_clr_sig(); lcd_symbol_set_group(LCD_SYMBOL_INDEX_ANT_FOOT, nmb+1)
#define lcd_symbol_antenna_disable()    lcd_symbol_antenna_set(LCD_SYMBOL_ANTENNA_DIS)

// Misc macros
#define lcd_symbol_set_all()            lcd_symbol_set_group(LCD_SYMBOL_INDEX_RAVEN, sizeof(lcd_symbol_chart))
#define lcd_symbol_clr_all()            lcd_symbol_clr_group(LCD_SYMBOL_INDEX_RAVEN, sizeof(lcd_symbol_chart))

/*========================= TYPEDEFS                 =========================*/
//! Type used for selection padding type (argument in some LCD functions)
typedef enum {
    LCD_NUM_PADDING_ZERO,
    LCD_NUM_PADDING_SPACE
} lcd_padding_t;


/*! \brief  Type used to set/clear symbols
 * 
 *          The values of this enumerator is built up of bit offset and memory
 *          offset for the symbol in the LCD memory
 */

typedef enum {
/*  name               = (bit_number << bit_number_offset) | mem_offset*/
    // Raven
    LCD_SYMBOL_RAVEN   = (7<<5) |  3,

    // Audio
    LCD_SYMBOL_BELL    = (2<<5) | 17,
    LCD_SYMBOL_TONE    = (6<<5) | 17,
    LCD_SYMBOL_MIC     = (3<<5) |  3,
    LCD_SYMBOL_SPEAKER = (2<<5) | 18,

    // Status
    LCD_SYMBOL_KEY     = (3<<5) |  2,
    LCD_SYMBOL_ATT     = (7<<5) |  2,

    // Time
    LCD_SYMBOL_SUN     = (6<<5) | 13,
    LCD_SYMBOL_MOON    = (6<<5) |  3,
    LCD_SYMBOL_AM      = (2<<5) | 15,
    LCD_SYMBOL_PM      = (6<<5) | 15,

    // Radio commuication status
    LCD_SYMBOL_RX      = (6<<5) | 18,
    LCD_SYMBOL_TX      = (5<<5) | 13,
    LCD_SYMBOL_IP      = (7<<5) | 13,
    LCD_SYMBOL_PAN     = (7<<5) | 18,
    LCD_SYMBOL_ZLINK   = (5<<5) |  8,
    LCD_SYMBOL_ZIGBEE  = (5<<5) |  3,

    // Antenna status
    LCD_SYMBOL_ANT_FOOT= (5<<5) | 18,
    LCD_SYMBOL_ANT_SIG1= (3<<5) |  0,
    LCD_SYMBOL_ANT_SIG2= (7<<5) |  0,
    LCD_SYMBOL_ANT_SIG3= (3<<5) |  1,
    LCD_SYMBOL_ANT_DIS = (7<<5) |  1,

    // Battery status
    LCD_SYMBOL_BAT_CONT= (4<<5) | 18,
    LCD_SYMBOL_BAT_CAP1= (4<<5) |  3,
    LCD_SYMBOL_BAT_CAP2= (4<<5) |  8,
    LCD_SYMBOL_BAT_CAP3= (4<<5) | 13,

    // Envelope status
    LCD_SYMBOL_ENV_OP  = (6<<5) |  8,
    LCD_SYMBOL_ENV_CL  = (0<<5) |  4,
    LCD_SYMBOL_ENV_MAIN= (2<<5) |  4,

    // Temperature
    LCD_SYMBOL_C       = (6<<5) | 16,
    LCD_SYMBOL_F       = (2<<5) | 16,

    // Numeric
    LCD_SYMBOL_MINUS   = (7<<5) |  8,
    LCD_SYMBOL_DOT     = (4<<5) |  4,
    LCD_SYMBOL_COL     = (6<<5) |  4,
} lcd_symbol_t;

//! Type used for set/clear symbolsin the set/clear_group functions
typedef enum {
    // Raven
    LCD_SYMBOL_INDEX_RAVEN,

    // Audio
    LCD_SYMBOL_INDEX_BELL,
    LCD_SYMBOL_INDEX_TONE,
    LCD_SYMBOL_INDEX_MIC,
    LCD_SYMBOL_INDEX_SPEAKER,

    // Status
    LCD_SYMBOL_INDEX_KEY,
    LCD_SYMBOL_INDEX_ATT,

    // Time
    LCD_SYMBOL_INDEX_SUN,
    LCD_SYMBOL_INDEX_MOON,
    LCD_SYMBOL_INDEX_AM,
    LCD_SYMBOL_INDEX_PM,

    // Radio commuicati
    LCD_SYMBOL_INDEX_RX,
    LCD_SYMBOL_INDEX_TX,
    LCD_SYMBOL_INDEX_IP,
    LCD_SYMBOL_INDEX_PAN,
    LCD_SYMBOL_INDEX_ZLINK,
    LCD_SYMBOL_INDEX_ZIGBEE,

    // Antenna status
    LCD_SYMBOL_INDEX_ANT_FOOT,
    LCD_SYMBOL_INDEX_ANT_SIG1,
    LCD_SYMBOL_INDEX_ANT_SIG2,
    LCD_SYMBOL_INDEX_ANT_SIG3,
    LCD_SYMBOL_INDEX_ANT_DIS,

    // Battery status
    LCD_SYMBOL_INDEX_BAT_CONT,
    LCD_SYMBOL_INDEX_BAT_CAP1,
    LCD_SYMBOL_INDEX_BAT_CAP2,
    LCD_SYMBOL_INDEX_BAT_CAP3,

    // Envelope status
    LCD_SYMBOL_INDEX_ENV_OP,
    LCD_SYMBOL_INDEX_ENV_CL,
    LCD_SYMBOL_INDEX_ENV_MAIN,

    // Temperature
    LCD_SYMBOL_INDEX_C,
    LCD_SYMBOL_INDEX_F,

    // Numeric
    LCD_SYMBOL_INDEX_MINUS,
    LCD_SYMBOL_INDEX_DOT,
    LCD_SYMBOL_INDEX_COL,
} lcd_symbol_index_t;


//! Type used for selection default or low power LCD waveform type
typedef enum {
    LCD_WAVE_DEFAULT   = 0,
    LCD_WAVE_LOW_POWER = 1
} lcd_wave_t;


//! Type used for enable/disable LCD buffer to save power
typedef enum {
    LCD_BUFFER_ON  = 0,
    LCD_BUFFER_OFF = 1
} lcd_buffer_t;


//! Type used for enabe/disable blanking after each frame
typedef enum {
    LCD_BLANKING_OFF = 0,
    LCD_BLANKING_ON  = 1
} lcd_blanking_t;


//! Type used for selecting system/extern clock as LCD clock source
typedef enum {
    LCD_CLOCK_SYSTEM = 0,
    LCD_CLOCK_EXTERN = 1
} lcd_clock_t;


//! Type used for configuring the LCD bias
typedef enum {
    LCD_BIAS_THIRD = 0,
    LCD_BIAS_HALF  = 1
} lcd_bias_t;


//! Type used for setting LCD clock prescaler (affecting frame rate)
typedef enum {
    LCD_PRESCL_16   = 0x0,
    LCD_PRESCL_64   = 0x1,
    LCD_PRESCL_128  = 0x2,
    LCD_PRESCL_256  = 0x3,
    LCD_PRESCL_512  = 0x4,
    LCD_PRESCL_1024 = 0x5,
    LCD_PRESCL_2048 = 0x6,
    LCD_PRESCL_4096 = 0x7,
} lcd_prescl_t;


//! Type used for setting LCD clock divide factor (affecting frame rate)
typedef enum {
    LCD_DIV_1 = 0,
    LCD_DIV_2 = 1,
    LCD_DIV_3 = 2,
    LCD_DIV_4 = 3,
    LCD_DIV_5 = 4,
    LCD_DIV_6 = 5,
    LCD_DIV_7 = 6,
    LCD_DIV_8 = 7
} lcd_div_t;

//! Type used for configuring drive level
typedef enum {
    LCD_DRIVE_300  = 0x0,
    LCD_DRIVE_70   = 0x2,
    LCD_DRIVE_150  = 0x4,
    LCD_DRIVE_450  = 0x6,
    LCD_DRIVE_575  = 0x8,
    LCD_DRIVE_850  = 0xA,
    LCD_DRIVE_1150 = 0xC,
    LCD_DRIVE_HALF = 0xE,
    LCD_DRIVE_FULL = 0xF
} lcd_drive_t;

//! Type for setting contrast level
typedef enum {
    LCD_CONTRAST_2_60 = 0x0,
    LCD_CONTRAST_2_65 = 0x1,
    LCD_CONTRAST_2_70 = 0x2,
    LCD_CONTRAST_2_75 = 0x3,
    LCD_CONTRAST_2_80 = 0x4,
    LCD_CONTRAST_2_85 = 0x5,
    LCD_CONTRAST_2_90 = 0x6,
    LCD_CONTRAST_2_95 = 0x7,
    LCD_CONTRAST_3_00 = 0x8,
    LCD_CONTRAST_3_05 = 0x9,
    LCD_CONTRAST_3_10 = 0xA,
    LCD_CONTRAST_3_15 = 0xB,
    LCD_CONTRAST_3_20 = 0xC,
    LCD_CONTRAST_3_25 = 0xD,
    LCD_CONTRAST_3_30 = 0xE,
    LCD_CONTRAST_3_35 = 0xF
} lcd_contrast_t;

//! Type used to set scrolling speed
typedef enum {
    LCD_SCROLLING_FAST = 1,
    LCD_SCROLLING_MEDIUM = 2,
    LCD_SCROLLING_SLOW = 3
} lcd_scrolling_t;


//! Configuration struct (Matchting the AVR LCD contrller registers to speed up initialization)
typedef struct {
    union {
        struct {
            unsigned int    blanking: 1;
            unsigned int        : 1;
            unsigned int    buffer  : 1;
            unsigned int        : 3;
            unsigned int    wave    : 1;
            unsigned int        : 1;
        };
        uint8_t             lcdcra;
    };
    union {
        struct {
            unsigned int        : 6;
            unsigned int    bias    : 1;
            unsigned int    clock   : 1;
        };
        uint8_t             lcdcrb;
    };
    union {
        struct {
            unsigned int    div     : 3;
            unsigned int    res5    : 1;
            unsigned int    prescl  : 3;
            unsigned int    res6    : 1;
        };
        uint8_t             lcdfrr;
    };
    union {
        struct {
            unsigned int    contrast: 4;
            unsigned int    drive   : 4;
        };
        uint8_t             lcdccr;
    };
    lcd_scrolling_t scrolling;
} lcd_config_t;
/*========================= PUBLIC VARIABLES         =========================*/
//! Not a public variable, but needed for some of the macros
extern volatile unsigned char* lcd_seg_memory_base;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief          Initialization and configuration of the LCD driver
 *
 *
 *  Example:
 *  \code
 *      // Configuring LCD with Extern clock (TOSC, 32.768kHz)
 *      //                      32786 Hz          32786 Hz
 *      //  frame_rate = ------------------ = ------------- = 32 Hz
 *      //               8 * .prescl * .div     8 * 16 * 8
 *      //
 *      lcd_config_t lcd_config = {
 *          .wave       = LCD_WAVE_LOW_POWER,
 *          .buffer     = LCD_BUFFER_ON,
 *          .blanking   = LCD_BLANKING_OFF,
 *          .clock      = LCD_CLOCK_EXTERN,
 *          .bias       = LCD_BIAS_HALF,
 *          .prescl     = LCD_PRESCL_16,
 *          .div        = LCD_DIV_8,
 *          .drive      = LCD_DRIVE_450,
 *          .contrast   = LCD_CONTRAST_3_30
 *      };
 *
 *      lcd_init(lcd_config);
 *  \endcode
 *
 *
 * \param[in]		config			    LCD configuration (\ref lcd_config_t)
 *
 *
 * \return                              EOF on error
 */
int  lcd_init(lcd_config_t config);


/*! \brief          De-initialize the LCD driver
 *
 */
void lcd_deinit(void);


/*! \brief          Configure the LCD driver
 *
 *
 *  Example:
 *  \code
 *      // Configuring LCD with Extern clock (TOSC, 32.768kHz)
 *      //                      32786 Hz          32786 Hz
 *      //  frame_rate = ------------------ = ------------- = 32 Hz
 *      //               8 * .prescl * .div     8 * 16 * 8
 *      //
 *      lcd_config_t lcd_config = {
 *          .wave       = LCD_WAVE_LOW_POWER,
 *          .buffer     = LCD_BUFFER_ON,
 *          .blanking   = LCD_BLANKING_OFF,
 *          .clock      = LCD_CLOCK_EXTERN,
 *          .bias       = LCD_BIAS_HALF,
 *          .prescl     = LCD_PRESCL_16,
 *          .div        = LCD_DIV_8,
 *          .drive      = LCD_DRIVE_450,
 *          .contrast   = LCD_CONTRAST_3_30
 *      };
 *
 *      lcd_init(lcd_config);
 *  \endcode
 *
 *
 * \param[in]		config			    LCD configuration (\ref lcd_config_t)
 *
 *
 * \return                              EOF on error
 */
int  lcd_config_set(lcd_config_t config);


/*! \brief          Get current LCD configuration
 *
 *
 * \return                              EOF on error
 */
lcd_config_t lcd_config_get(void);


/*! \brief          Show a hexadecimal number in the numeric section
 *
 *                  
 *
 * \param[in]		numb			    Number to show
 * \param[in]		padding			    Select zero- or space padding
 *
 *
 * \return                              EOF on error
 */
int lcd_num_puthex(uint16_t numb, lcd_padding_t padding);


/*! \brief          Show a decimal number in the numeric section
 *
 *                  
 *
 * \param[in]		numb			    Number to show
 * \param[in]		padding			    Select zero- or space padding
 *
 *
 * \return                              EOF on error
 */
int lcd_num_putdec(int numb, lcd_padding_t padding);


/*! \brief          Clear a number in the numeric section
 *
 *                  
 *
 * \param[in]		digit			    Digit to clear 
 *
 *
 * \return                              EOF on error
 */
int lcd_num_clr(int8_t digit);


/*! \brief          Clear entire numeric section
 *
 *                  
 *
 * \return                              EOF on error
 */
int lcd_num_clr_all(void);


/*! \brief          Enable/disable digits in the numeric sesction
 *
 *                  
 *
 * \param[in]		digit_mask			Digits to enable (mask)
 *
 *
 * \return                              EOF on error
 */
int lcd_num_enable(int8_t digit_mask);


/*! \brief          Enable/disable digit blink in the numeric sesction
 *
 *                  
 *
 * \param[in]		digit_mask			Selected digits
 * \param[in]		start_value_mask	Start value (on/off)
 *
 *
 * \return                              EOF on error
 */
int lcd_num_blink_set(uint8_t digit_mask, uint8_t start_value_mask);


/*! \brief          Show a text in the alphanumeric section
 *
 *                  
 *
 * \param[in]		s			        String
 *
 *
 * \return                              EOF on error
 */
int  lcd_puts(const char* s);


/*! \brief          Show a text (stored in progmem) in the alphanumeric section
 *
 *                  
 *
 * \param[in]		s			        String  
 *
 *
 * \return                              EOF on error
 */
int  lcd_puts_P(PROGMEM_STRING_T s);


/*! \brief          Show data from an array in the alphanumeric section
 *
 *                  
 *
 * \param[in]		array	            Array holding data
 * \param[in]		size			    Size of array
 *
 *
 * \return                              EOF on error
 */
int  lcd_puta(const uint8_t array[], int size);


/*! \brief          Put a character in the alphanumeric section
 *
 *                  The character will be displayed in the next unused position as in the ANSI C putchar()
 *
 * \param[in]		c			        Character to put
 *
 *
 * \return                              EOF on error
 */
int  lcd_putchar(unsigned char c);


/*! \brief          Display a cursor in the alphanumeric section
 *
 *                  The cursor is a '_' in blank positions or positions with a ' '
 *                  In positions holding any other prinitng character the cursor is a ' '
 *
 * \param[in]		pos			        Position to put the cursor
 * \param[in]		start_state			Select  cursor start on or off
 *
 *
 * \return                              EOF on error
 */
int  lcd_cursor_set(int8_t pos, bool start_state);


/*! \brief          Clear the cursor
 *
 *                  
 *
 * \return                              EOF on error
 */
void lcd_cursor_clr(void);


/*! \brief          Enable a symbol
 *
 *                  The symbol is selected using the self explaining \ref lcd_symbol_t type
 *
 * \param[in]		symbol			    Selected symbol
 *
 *
 * \return                              EOF on error
 */
void lcd_symbol_set(lcd_symbol_t symbol);


/*! \brief          Enable a group of symbols
 *
 *                  The symbols is selected using the index enumerator \ref lcd_symbol_t type
 *                  and a count
 *
 * \param[in]		index			    Start (\ref lcd_symbol_index_t) of symbol group to set
 * \param[in]		count			    Number of symbols to set
 *
 *
 * \return                              EOF on error
 */
void lcd_symbol_set_group(lcd_symbol_index_t index, int count);


/*! \brief          Clear a symbol
 *
 *                  The symbol is selected using the self explaining \ref lcd_symbol_t type
 *
 * \param[in]		symbol			    Selected symbol
 *
 *
 * \return                              EOF on error
 */
void lcd_symbol_clr(lcd_symbol_t symbol);


/*! \brief          Clear a group of symbols
 *
 *                  The symbols is selected using the index enumerator \ref lcd_symbol_t type
 *                  and a count
 *
 * \param[in]		index			    Start (\ref lcd_symbol_index_t) of symbol group to clear
 * \param[in]		count			    Number of symbols to clear
 *
 *
 * \return                              EOF on error
 */
void lcd_symbol_clr_group(lcd_symbol_index_t index, int count);


/*! \brief          Shift alphanumeric text one position left
 */
extern void lcd_text_shift_left(void);


/*! \brief          Toggle cursor in current position
 */
extern void lcd_cursor_toggle(void);


/*! \brief          Refresh numeric section
 */
extern void lcd_num_refresh(void);
//! @}
#endif // __LCD_H__
/*EOF*/
