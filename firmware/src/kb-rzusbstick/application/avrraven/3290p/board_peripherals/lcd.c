// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Driver for the AVR Raven LCD display.
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
 * $Id: lcd.c 41576 2008-05-13 14:10:44Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "lcd.h"

//! \addtogroup grLCDLowLevel
//! @{
/*========================= MACROS                   =========================*/
///{ Constant defines for making seg_map[] table
/*
 -- a --
|       |
f       b
|       |
|-- g --|
|       |
e       c
|       |
 -- d --
*/
#define NS_A 0x01
#define NS_B 0x02
#define NS_C 0x04
#define NS_D 0x08
#define NS_E 0x10
#define NS_F 0x20
#define NS_G 0x40
///}


///{ Constant defines for making LCD_character_table[] table
/*
 -- a --
|\  |  /|
f h i j b
|  \|/  |
|-g1-g2-|
|  /|\  |
e m l k c
|/  |  \|
 -- d --
*/
#define AS_A    0x0004
#define AS_B    0x0080
#define AS_C    0x8000
#define AS_D    0x1000
#define AS_E    0x0100
#define AS_F    0x0001
#define AS_G1   0x0010
#define AS_G2   0x0800
#define AS_H    0x0002
#define AS_I    0x0020
#define AS_J    0x0040
#define AS_K    0x0400
#define AS_L    0x2000
#define AS_M    0x0200
///}

///{ Constant defines for indexing seg_map[] table
#define LCD_SEV_SEG_INDEX_0     (0)
#define LCD_SEV_SEG_INDEX_1     (1)
#define LCD_SEV_SEG_INDEX_2     (2)
#define LCD_SEV_SEG_INDEX_3     (3)
#define LCD_SEV_SEG_INDEX_4     (4)
#define LCD_SEV_SEG_INDEX_5     (5)
#define LCD_SEV_SEG_INDEX_6     (6)
#define LCD_SEV_SEG_INDEX_7     (7)
#define LCD_SEV_SEG_INDEX_8     (8)
#define LCD_SEV_SEG_INDEX_9     (9)
#define LCD_SEV_SEG_INDEX_A     (10)
#define LCD_SEV_SEG_INDEX_B     (11)
#define LCD_SEV_SEG_INDEX_C     (12)
#define LCD_SEV_SEG_INDEX_D     (13)
#define LCD_SEV_SEG_INDEX_E     (14)
#define LCD_SEV_SEG_INDEX_F     (15)
#define LCD_SEV_SEG_INDEX_SPACE (16)
#define LCD_SEV_SEG_INDEX_MINUS (17)
///}


/*========================= TYPEDEFS                 =========================*/
//! Type used to set LCD duty cycle
typedef enum {
    LCD_DUTY_STATIC = 0,
    LCD_DUTY_HALF   = 1,
    LCD_DUTY_THIRD  = 2,
    LCD_DUTY_QUART  = 3
} lcd_duty_t;

//! Type used to set LCD port muxing
typedef enum {
    LCD_PM_0_12 = 0x0,
    LCD_PM_0_14 = 0x1,
    LCD_PM_0_16 = 0x2,
    LCD_PM_0_18 = 0x3,
    LCD_PM_0_20 = 0x4,
    LCD_PM_0_22 = 0x5,
    LCD_PM_0_23 = 0x6,
    LCD_PM_0_24 = 0x7,
    LCD_PM_0_26 = 0x8,
    LCD_PM_0_28 = 0x9,
    LCD_PM_0_30 = 0xA,
    LCD_PM_0_32 = 0xB,
    LCD_PM_0_34 = 0xC,
    LCD_PM_0_36 = 0xD,
    LCD_PM_0_38 = 0xE,
    LCD_PM_0_39 = 0xF
} lcd_pm_t;
/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
///{ Mapping of segments for different characters
PROGMEM_DECLARE(static const unsigned char)seg_map[] = {
        NS_A|NS_B|NS_C|NS_D|NS_E|NS_F     , // 0
             NS_B|NS_C                    , // 1
        NS_A|NS_B|     NS_D|NS_E|     NS_G, // 2
        NS_A|NS_B|NS_C|NS_D|          NS_G, // 3
             NS_B|NS_C|          NS_F|NS_G, // 4
        NS_A|     NS_C|NS_D|     NS_F|NS_G, // 5
        NS_A|     NS_C|NS_D|NS_E|NS_F|NS_G, // 6
        NS_A|NS_B|NS_C                    , // 7
        NS_A|NS_B|NS_C|NS_D|NS_E|NS_F|NS_G, // 8
        NS_A|NS_B|NS_C|NS_D|     NS_F|NS_G, // 9
        NS_A|NS_B|NS_C|     NS_E|NS_F|NS_G, // A
                  NS_C|NS_D|NS_E|NS_F|NS_G, // B
        NS_A|          NS_D|NS_E|NS_F     , // C
             NS_B|NS_C|NS_D|NS_E|     NS_G, // D
        NS_A|          NS_D|NS_E|NS_F|NS_G, // E
        NS_A|               NS_E|NS_F|NS_G, // F
                                         0, // ' ' (space)
                                      NS_G  //  - (minus)
};	
    
//! Look-up tables for 14-segment characters
PROGMEM_DECLARE(static const unsigned int)LCD_character_table[] = // Character definitions table.
{//	(AS_A|AS_B|AS_C|AS_D|AS_E|AS_F|AS_G1|AS_G2|AS_H|AS_I|AS_J|AS_K|AS_L|AS_M),      // Description
	(                                                                      0),		// ' '
	(                                                                      0),		// '!' (Not defined)
	(                                                                      0),		// '"' (Not defined)
	(                                                                      0),		// '#' (Not defined)
	(                                                                      0),		// '$' (Not defined)
	(                                                                      0),		// '%' (Not defined)
	(                                                                      0),		// '&' (Not defined)
	(                                                                      0),		// ''' (Not defined)
	(                                                                      0),		// '(' (Not defined)
	(                                                                      0),		// ')' (Not defined)
	(                              AS_G1|AS_G2|AS_H|AS_I|AS_J|AS_K|AS_L|AS_M),		// '*'
	(                              AS_G1|AS_G2|     AS_I|          AS_L     ),      // '+'
	(                                                                      0),		// ',' (Not defined)
	(                              AS_G1|AS_G2                              ),		// '-'
	(                                                                      0),		// '.' (Not defined)
	(                                                    AS_J|          AS_M),		// '/'
	(AS_A|AS_B|AS_C|AS_D|AS_E|AS_F                                          ),		// '0'
	(     AS_B|AS_C                                                         ),		// '1'
	(AS_A|AS_B|     AS_D|AS_E|     AS_G1|AS_G2                              ),		// '2'
	(AS_A|AS_B|AS_C|AS_D|          AS_G1|AS_G2                              ),		// '3'
	(     AS_B|AS_C|          AS_F|AS_G1|AS_G2                              ),		// '4'
	(AS_A|     AS_C|AS_D     |AS_F|AS_G1|AS_G2                              ),		// '5'
	(AS_A|     AS_C|AS_D|AS_E|AS_F|AS_G1|AS_G2                              ),		// '6'
	(AS_A|AS_B|AS_C                                                         ),		// '7'
	(AS_A|AS_B|AS_C|AS_D|AS_E|AS_F|AS_G1|AS_G2                              ),		// '8'
	(AS_A|AS_B|AS_C|AS_D|     AS_F|AS_G1|AS_G2                              ),		// '9'
	(                                                                      0),		// ':' (Not defined)
	(                                                                      0),		// ';' (Not defined)
	(                                                    AS_J|AS_K          ),		// '<'
	(                                                                      0),		// '=' (Not defined)
	(                                          AS_H|                    AS_M),		// '>'
	(AS_A|                                               AS_J|     AS_L     ),		// '?'
	(AS_A|AS_B|     AS_D|AS_E|AS_F|      AS_G2|     AS_I                    ),		// '@'
	(AS_A|AS_B|AS_C|     AS_E|AS_F|AS_G1|AS_G2                              ),		// 'A' (+ 'a')
	(AS_A|AS_B|AS_C|AS_D|                AS_G2|     AS_I|          AS_L     ),		// 'B' (+ 'b')
	(AS_A|          AS_D|AS_E|AS_F                                          ),		// 'C' (+ 'c')
	(AS_A|AS_B|AS_C|AS_D|                           AS_I|          AS_L     ),		// 'D' (+ 'd')
	(AS_A|          AS_D|AS_E|AS_F|AS_G1|AS_G2                              ),		// 'E' (+ 'e')
	(AS_A|               AS_E|AS_F|AS_G1                                    ),		// 'F' (+ 'f')
	(AS_A|     AS_C|AS_D|AS_E|AS_F|      AS_G2                              ),		// 'G' (+ 'g')
	(     AS_B|AS_C|     AS_E|AS_F|AS_G1|AS_G2                              ),		// 'H' (+ 'h')
	(                                               AS_I|          AS_L     ),		// 'I' (+ 'i')
	(     AS_B|AS_C|AS_D|AS_E                                               ),		// 'J' (+ 'j')
	(                    AS_E|AS_F|AS_G1|                AS_J|AS_K          ),		// 'K' (+ 'k')
	(               AS_D|AS_E|AS_F                                          ),		// 'L' (+ 'l')
	(     AS_B|AS_C|     AS_E|AS_F|            AS_H|     AS_J               ),		// 'M' (+ 'm')
	(     AS_B|AS_C|     AS_E|AS_F|            AS_H|          AS_K          ),		// 'N' (+ 'n')
	(AS_A|AS_B|AS_C|AS_D|AS_E|AS_F                                          ),		// 'O' (+ 'o')
	(AS_A|AS_B|          AS_E|AS_F|AS_G1|AS_G2                              ),		// 'P' (+ 'p')
	(AS_A|AS_B|AS_C|AS_D|AS_E|AS_F|                           AS_K          ),		// 'Q' (+ 'q')
	(AS_A|AS_B|          AS_E|AS_F|AS_G1|AS_G2|               AS_K          ),		// 'R' (+ 'r')
	(AS_A|     AS_C|AS_D|     AS_F|AS_G1|AS_G2                              ),		// 'S' (+ 's')
	(AS_A|                                          AS_I|          AS_L     ),		// 'T' (+ 't')
	(     AS_B|AS_C|AS_D|AS_E|AS_F                                          ),		// 'U' (+ 'u')
	(                    AS_E|AS_F|                      AS_J|          AS_M),		// 'V' (+ 'v')
	(     AS_B|AS_C|     AS_E|AS_F|                           AS_K|     AS_M),		// 'W' (+ 'w')
	(                                          AS_H|     AS_J|AS_K|     AS_M),		// 'X' (+ 'x')
	(                                          AS_H|     AS_J|     AS_L     ),		// 'Y' (+ 'y')
	(AS_A|          AS_D|                                AS_J|          AS_M),		// 'Z' (+ 'z')
	(AS_A|          AS_D|AS_E|AS_F                                          ),		// '['
	(                                          AS_H|          AS_K          ),		// '\'
	(AS_A|AS_B|AS_C|AS_D                                                    ),		// ']'
	(                         AS_F|            AS_H                         ),		// '^'
	(               AS_D                                                    ),		// '_'
};


//! Look-up tables for 7-segment characters
PROGMEM_DECLARE(static const unsigned char) seg_inf[] = {
        2<<5|19, // A
        1<<5|19, // B
        1<<5|9,  // C
        2<<5|4,  // D
        2<<5|9,  // E
        2<<5|14, // F
        1<<5|14  // G
};

//! Symbol chart for all symbolic segment
PROGMEM_DECLARE(static const lcd_symbol_t) lcd_symbol_chart[LCD_SYMBOL_COUNT] = {
    // Raven
    LCD_SYMBOL_RAVEN   ,

    // Audio
    LCD_SYMBOL_BELL    ,
    LCD_SYMBOL_TONE    ,
    LCD_SYMBOL_MIC     ,
    LCD_SYMBOL_SPEAKER ,

    // Status
    LCD_SYMBOL_KEY     ,
    LCD_SYMBOL_ATT     ,

    // Time
    LCD_SYMBOL_SUN     ,
    LCD_SYMBOL_MOON    ,
    LCD_SYMBOL_AM      ,
    LCD_SYMBOL_PM      ,

    // Radio
    LCD_SYMBOL_RX      ,
    LCD_SYMBOL_TX      ,
    LCD_SYMBOL_IP      ,
    LCD_SYMBOL_PAN     ,
    LCD_SYMBOL_ZLINK   ,
    LCD_SYMBOL_ZIGBEE  ,

    // Antenna status
    LCD_SYMBOL_ANT_FOOT,
    LCD_SYMBOL_ANT_SIG1,
    LCD_SYMBOL_ANT_SIG2,
    LCD_SYMBOL_ANT_SIG3,
    LCD_SYMBOL_ANT_DIS ,

    // Battery status
    LCD_SYMBOL_BAT_CONT,
    LCD_SYMBOL_BAT_CAP1,
    LCD_SYMBOL_BAT_CAP2,
    LCD_SYMBOL_BAT_CAP3,

    // Envelope status
    LCD_SYMBOL_ENV_OP  ,
    LCD_SYMBOL_ENV_CL  ,
    LCD_SYMBOL_ENV_MAIN,

    // Temperature
    LCD_SYMBOL_C       ,
    LCD_SYMBOL_F       ,

    // Numeric
    LCD_SYMBOL_MINUS   ,
    LCD_SYMBOL_DOT     ,
    LCD_SYMBOL_COL
};

//! LCD text buffer. 
static unsigned char lcd_text[LCD_DATA_BUFFER_SIZE];

//! Text buffer read pointer for text field in LCD display. When ptr>0 characters in front will be cleared (space)
static int lcd_text_rd_ptr = 0;

//! Text pointer for writing new chars to text buffer
static int lcd_text_wr_ptr = 0;

//! Flag set when text scrolling in the alphanumeric section is necessary
static bool lcd_scroll_enable;


#ifndef LCD_DRIVER_MINIMAL
//! LCD display buffer (actual text showed in display)
static unsigned char lcd_buffer[LCD_ALPHA_DIGIT_COUNT];

//! Scrolling prescaler buffer
static int lcd_scroll_prescale;

//! Scrolling prescaler value
static int lcd_scroll_prescale_value;

//! Cursor blink prescaler buffer
static int lcd_cursor_prescale;

//! Cursor blink prescaler value
static int lcd_cursor_prescale_value;

//! Numeric segment blink prescaler buffer
static int lcd_num_blink_prescale;

//! Numeric segment blink prescaler value
static int lcd_num_blink_prescale_value;

//! Flag for cursor enale/disable
static bool lcd_cursor_enable;

//! Mask for current enabled numeric digits
static uint8_t lcd_num_enable_mask;

//! Mask for current enabled numeric digits
static uint8_t lcd_num_blink_enable_mask;

//! Current cursor position
static int8_t lcd_cursor_pos;

//! Flag indication current active char visible or not
static bool active_char_visible;
#endif

//! Buffer for numeric data
static uint16_t lcd_num_data;

//! Current padding type (\ref lcd_padding_t)
static lcd_padding_t lcd_num_padding;

//! Indicator for negative number
static bool lcd_num_negative;

//! Mask for current disabled numeric digits
static uint8_t lcd_num_disable_mask;

//! Pointer to LCD data memory base
volatile unsigned char* lcd_seg_memory_base = (volatile unsigned char*)0xEC;

/*========================= PUBLIC FUNCTIONS         =========================*/
    
/*========================= PRIVATE FUNCTIONS        =========================*/
//! Low level printing to numeric section
static int  lcd_num_print(uint16_t numb, bool negative, lcd_padding_t padding);

//! Low level printing of a specific in numeric section
static void lcd_nmb_print_dig(uint8_t val, int dig);

//! Low level text put
static int  lcd_text_put(const char* s, int pos);

//! Low level character put
static int  lcd_char_put(unsigned char c, int pos);

//! Flag indicating driver initialized
static bool lcd_initialized = false;


/*========================= IMPLEMENTATION           =========================*/
int lcd_init(lcd_config_t config)
{
    // If allready initialized, deinitialize first
    if (lcd_initialized == true) {
        lcd_deinit();
    }
    
    // Enable module
    PRR &= ~(1 << PRLCD);

    // Configure and enable LCD controller
    LCDCRB = config.lcdcrb|(LCD_PM_0_39<<LCDPM0)|(LCD_DUTY_QUART<<LCDMUX0); // Add port mask/mux
    LCDFRR = config.lcdfrr;
    LCDCCR = config.lcdccr;
    LCDCRA = config.lcdcra|(1<<LCDEN)|(1<<LCDIE); // Add interrupt- and LCD- enable
    
    // clear screen
    lcd_symbol_clr_all();

    // Set scrolling value
    
#ifndef LCD_DRIVER_MINIMAL
    lcd_scroll_prescale_value = config.scrolling;
    lcd_scroll_prescale = lcd_scroll_prescale_value;
    
    // Set numeric section blink frquency
    lcd_num_blink_prescale_value = 8;
    lcd_num_blink_prescale = lcd_num_blink_prescale_value;
    
    // Set text cursor/blink frquency
    lcd_cursor_prescale_value = 8;
    lcd_cursor_prescale = lcd_cursor_prescale_value;
    
    // Enable all numeric digits, without blinking
    lcd_num_enable_mask = 0x0F;
    lcd_num_blink_enable_mask = 0x00;
#endif

    // Indicate successful initialization
    lcd_initialized = true;
    
    return 0;
}

void lcd_deinit(void) {
    PRR |= (1 << PRLCD);
    
    lcd_initialized = false;
}


int lcd_config_set(lcd_config_t config)
{
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }
    
    // Save LCD controller state (interrupt and enable bit)
    uint8_t lcd_state = LCDCRA & ((1<<LCDEN)|(1<<LCDIE));
    
    // Set scrolling value (software)
#ifndef LCD_DRIVER_MINIMAL
    lcd_scroll_prescale_value = config.scrolling;
#endif
    
    // Configure and enable LCD controller
    LCDCRB = config.lcdcrb|(LCD_PM_0_39<<LCDPM0)|(LCD_DUTY_QUART<<LCDMUX0); // Add port mask/mux
    LCDFRR = config.lcdfrr;
    LCDCCR = config.lcdccr;
    LCDCRA = config.lcdcra|lcd_state; // Set previous interrupt- and LCD- enable state
    
    return 0;
}

lcd_config_t lcd_config_get(void)
{
    lcd_config_t config;
    
    config.lcdcrb = LCDCRB;
    config.lcdfrr = LCDFRR;
    config.lcdccr = LCDCCR;
    config.lcdcra = LCDCRA;
    
    return config;
}

int lcd_num_puthex(uint16_t numb, lcd_padding_t padding)
{
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }
    
    //
    return lcd_num_print(numb, false, padding);
}

int lcd_num_putdec(int numb, lcd_padding_t padding)
{
    uint16_t bcd;
    
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }

    // check for overflow
    if (numb > 9999) {
        numb = 9999;
    } else if (numb < -9999) {
        numb = -9999;
    }

    // convert to BCD
    bcd = itobcd((uint16_t)abs(numb));

    // Print
    return lcd_num_print(bcd, (bool)(numb<0), padding);
}

int lcd_num_clr_all(void)
{
	volatile unsigned char* lcd_data = lcd_seg_memory_base;

    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }
    
    for (int i=0;i<4;++i){
        for (int j=0;j<LCD_ALPHA_DIGIT_COUNT;++j){
            lcd_data[seg_inf[j]&0x1F] &= ~((seg_inf[j]>>5)<<(i*2));
        }
    }
    return 0;
}

#ifndef LCD_DRIVER_MINIMAL
int lcd_num_enable(int8_t digit_mask)
{
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }
    
    lcd_num_enable_mask = digit_mask;
    
    return 0;
}

int lcd_num_blink_set(uint8_t digit_mask, uint8_t start_value_mask)
{
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }
    
    // Enable blinking on all enabled digits
    lcd_num_blink_enable_mask = digit_mask & lcd_num_enable_mask;
    
    // Disable all digits that is not enabled, or set in start_value_mask
    lcd_num_disable_mask = ~lcd_num_enable_mask | (start_value_mask & digit_mask);
    
    // Reset prescaler
    lcd_num_blink_prescale = lcd_num_blink_prescale_value;
    
    // Update numeric display
    lcd_num_print(lcd_num_data, lcd_num_negative, lcd_num_padding);
    
    return 0;
}

void lcd_cursor_clr(void)
{
    lcd_cursor_enable = false;
}

int lcd_cursor_set(int8_t pos, bool start_state)
{
    // Return error if trying to set cursor outside display range
    if ((pos<0) || (pos>=LCD_ALPHA_DIGIT_COUNT)) {
        return EOF;
    }
    
    // Store start value for cursor on/off
    active_char_visible = start_state;
    
    // Set cursor position/enable flag, and update cursor emediately
    lcd_cursor_enable = true;
    lcd_cursor_pos = pos;
    lcd_cursor_toggle();
    
    // Reset prescaler
    lcd_cursor_prescale = lcd_cursor_prescale_value;
    
    // Return success
    return 0;
}

void lcd_cursor_toggle(void)
{
    if (active_char_visible == true) {
        lcd_char_put(lcd_buffer[lcd_cursor_pos], (int)lcd_cursor_pos);
        active_char_visible = false;
    } else {
        unsigned char temp_char = (lcd_buffer[lcd_cursor_pos] == ' ') ? '_' : ' ';
        lcd_char_put(temp_char, (int)lcd_cursor_pos);
        active_char_visible = true;
    }
}
#endif


int lcd_num_clr(int8_t digit)
{
	volatile unsigned char* lcd_data = lcd_seg_memory_base;
    
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }
    
    // Clear selected numeric digit
    for (int j=0;j<LCD_ALPHA_DIGIT_COUNT;++j){
        lcd_data[seg_inf[j]&0x1F] &= ~((seg_inf[j]>>5)<<(digit*2));
    }
    
    // Return success
    return 0;
}

int lcd_puts(const char* s)
{
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }
    
    //
    strcpy((char*)lcd_text, s);
    lcd_text_wr_ptr = strlen(s);
    lcd_text_rd_ptr = 0;
    
    lcd_scroll_enable = false;
    
    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], 0);
    
    lcd_scroll_enable = (lcd_text_wr_ptr > LCD_ALPHA_DIGIT_COUNT) ? true : false;
    
    return 0;
}

int lcd_puta(const uint8_t array[], int size)
{
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }

    lcd_scroll_enable = false;
    
    size_t actual_size = (size>(LCD_DATA_BUFFER_SIZE-1)) ? LCD_DATA_BUFFER_SIZE-1 : size;
    memcpy((void*)lcd_text, (void const*)array, actual_size);
    lcd_text[actual_size] = '\0'; // zero terminate data
    lcd_text_wr_ptr = size;
    lcd_text_rd_ptr = 0;
    
    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], 0);

    lcd_scroll_enable = (lcd_text_wr_ptr > LCD_ALPHA_DIGIT_COUNT) ? true : false;

    return 0;
}

int lcd_puts_P(PROGMEM_STRING_T s)
{
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }

    lcd_scroll_enable = false;
    
    int str_len = strlen_P(s);
    size_t cpy_len =  str_len+1;
    strncpy_P((char*)lcd_text, s, cpy_len);
    lcd_text_wr_ptr = str_len;
    lcd_text_rd_ptr = 0;

    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], 0);

    lcd_scroll_enable = (lcd_text_wr_ptr > LCD_ALPHA_DIGIT_COUNT) ? true : false;

    return 0;
}

int lcd_putchar(unsigned char c)
{
    // Return if not initialized
    if (lcd_initialized == false) {
        return EOF;
    }

    lcd_scroll_enable = false;
    
    lcd_text[lcd_text_wr_ptr++] = c;
    lcd_text[lcd_text_wr_ptr] = 0;

    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], 0);

    lcd_scroll_enable = (lcd_text_wr_ptr > LCD_ALPHA_DIGIT_COUNT) ? true : false;

    return 0;
}

void lcd_symbol_set(lcd_symbol_t symbol)
{
    unsigned char mem_offset;
    unsigned char bit_offset;
    volatile unsigned char* lcd_data = lcd_seg_memory_base;

    // symbol format = bbbnnnnn where b is bit and n is offset
    bit_offset = (symbol >> 5);
    mem_offset = (symbol & 0x1F);
    if ( mem_offset >= 20 ){
        return;  //data out of range of the LCD registers
    }
    lcd_data = lcd_data + mem_offset;	// Point to the  relevant LCDDR

    *lcd_data = *lcd_data | ( 1 << bit_offset);
}


void lcd_symbol_clr(lcd_symbol_t symbol)
{
    unsigned char offset;
    unsigned char setbit;
    volatile unsigned char* lcd_data = lcd_seg_memory_base;

    // symbol format = bbbnnnnn where b is bit and n is offset
    setbit = (symbol >> 5);
    offset = (symbol & 0x1F);
    if ( offset >= 20 ){
        return;  //data out of range of the LCD registers
    }

    lcd_data = lcd_data + offset;	// Point to the  relevant LCDDR

    *lcd_data = *lcd_data & ~( 1 << setbit);
}

void lcd_symbol_set_group(lcd_symbol_index_t index, int count)
{
    count = (index + count)>sizeof(lcd_symbol_chart) ?
        sizeof(lcd_symbol_chart) - index :
        count;

    for( int i=index; i<index+count; ++i){
        lcd_symbol_set(lcd_symbol_chart[i]);
    }
}

void lcd_symbol_clr_group(lcd_symbol_index_t index, int count)
{
    count = (index + count)>sizeof(lcd_symbol_chart) ?
        sizeof(lcd_symbol_chart) - index :
        count;

    for( int i=index; i<index+count; ++i){
        lcd_symbol_clr(lcd_symbol_chart[i]);
    }
}

static int lcd_num_print(uint16_t numb, bool negative, lcd_padding_t padding)
{
    // Save arguments
    lcd_num_data = numb;
    lcd_num_padding = padding;
    lcd_num_negative = negative;
    
    // Print number
    for (int i=0;i<4;/**/) {
        // get segments for this digit and print it
        lcd_nmb_print_dig(seg_map[(numb&(0xF<<4*i))>>4*i], i);

        // if rest of number is zero
        if (++i<4) {
            if (numb >> 4*i == 0) {
                if (negative == true) { // print a 'minus' in the next digit field
                    lcd_nmb_print_dig(seg_map[(padding == LCD_NUM_PADDING_ZERO) ? LCD_SEV_SEG_INDEX_0 : LCD_SEV_SEG_INDEX_MINUS], i++);
                    if (padding == LCD_NUM_PADDING_ZERO) {
                        lcd_symbol_set(LCD_SYMBOL_MINUS);
                    }
                } else {
                    lcd_symbol_clr(LCD_SYMBOL_MINUS);
                }
                while (i<4){
                    lcd_nmb_print_dig(seg_map[(padding == LCD_NUM_PADDING_ZERO) ? LCD_SEV_SEG_INDEX_0 : LCD_SEV_SEG_INDEX_SPACE], i++);
                }
            }
        } else {
            if (negative == true) {
                lcd_symbol_set(LCD_SYMBOL_MINUS);
            } else {
                lcd_symbol_clr(LCD_SYMBOL_MINUS);
            }
        }
    }   
    return 0;
}

static void lcd_nmb_print_dig(uint8_t val, int dig)
{
	volatile unsigned char* lcd_data = lcd_seg_memory_base;

    // If digit is disabled, print a space
    if (lcd_num_disable_mask & (1<<dig)){
        val = seg_map[LCD_SEV_SEG_INDEX_SPACE];
    }
    
    for (int j=0;j<LCD_ALPHA_DIGIT_COUNT;++j){
        if (val & (1<<j)) {
            lcd_data[seg_inf[j]&0x1F] |= (seg_inf[j]>>5)<<(dig*2);
        }
        else {
            lcd_data[seg_inf[j]&0x1F] &= ~((seg_inf[j]>>5)<<(dig*2));
        }
    }
}

void lcd_num_refresh(void)
{
    lcd_num_print(lcd_num_data, lcd_num_negative, lcd_num_padding);
}

void lcd_text_shift_left(void)
{
    static int pos = 1;
    
    // Return if scrolling is been disabled after event has been posted
    if (lcd_scroll_enable == false) {
        return;
    }
    
    if (lcd_text[lcd_text_rd_ptr] == 0) {
        lcd_text_rd_ptr = 0;
        pos = LCD_ALPHA_DIGIT_COUNT-1;
    }
    
    else {
        if (pos){
            pos--;
        }
        else {
            lcd_text_rd_ptr++;
        }
    }

    lcd_text_put((char*)&lcd_text[lcd_text_rd_ptr], pos);
    return;
}

static int lcd_text_put(const char* s, int pos)
{
    int i;

    // pad with spaces in front if offset > 0
    for (i=0; i<pos; i++) {
         lcd_char_put(' ', i);
    }

    // print characters, overwrite with spaces at end if necessary
    for ( i=pos; i<LCD_ALPHA_DIGIT_COUNT; ++i) {
#ifndef LCD_DRIVER_MINIMAL
            lcd_buffer[i] = *s;
#endif
        if (*s == 0) {
            lcd_char_put(' ', i);
        }
        else {
            lcd_char_put( (unsigned char)*s++, i);
        }
    }
    return 0;
}

static int lcd_char_put(unsigned char c, int pos)
{
	unsigned int seg, segMask;
	unsigned char i;
	unsigned char mask, nibble, nibbleMask;

	volatile unsigned char* lcd_data = lcd_seg_memory_base;
    unsigned char lcd_reg;

    if ((pos < 0) || (pos >= LCD_ALPHA_DIGIT_COUNT)){
      return EOF;
    }

	// c is in character_table?
	if ((c >= ' ') && (c <= 'z'))
	{
        //  Convert to upper if necessary.
		if (c >= 'a' ){
            c &= ~0x20;
        }
        
        // Subtract table offset (ascii characters before space is not precent in table)
        c -= ' ';
        
        // Return if table index (c) points outside array
		if (c > sizeof(LCD_character_table)/sizeof(int)){
		   return EOF;
		}
        
        // Else, look-up char
	    seg = LCD_character_table[c];
	}
	else {
		return EOF;		//ASCII code out of range
	}


	// Adjust mask according to digit
	segMask = 0x4008;  // masking out two bits

	i = pos;		//i used as pointer offset
	i >>= 1;
	lcd_data += i;	// Point to the first relevant LCDDR; i = {0,0,1,1,2,2}

	i = 4;			//i used as loop counter
	do{
        nibble = seg & 0x000F;
        nibbleMask = segMask & 0x000F;

        seg >>= 4;
        segMask >>= 4;

        if ((pos+1) & 0x01) {
            mask = 0xF0 | nibbleMask;
        }
        else {
            nibble <<= 4;
            mask = 0x0F | ( nibbleMask <<4 );
        }
        lcd_reg = *lcd_data;
        *lcd_data |= (lcd_reg & mask) | nibble;  //write new bit values

        lcd_reg = *lcd_data;
        *lcd_data &= (lcd_reg & mask) | nibble;

        lcd_data += 5;
    } while ( --i );

    return 0;
}

ISR(LCD_vect)
{
#ifndef LCD_DRIVER_MINIMAL
    if (lcd_scroll_enable == true) {
        if (--lcd_scroll_prescale == 0) {
            LCD_SHIFT_LEFT;
            lcd_scroll_prescale = lcd_scroll_prescale_value;
        }
    } else if (lcd_cursor_enable == true) {
        if (--lcd_cursor_prescale == 0) {
            LCD_CURSOR_TOGGLE;
            lcd_cursor_prescale = lcd_cursor_prescale_value;
        }
    }
    
    if (lcd_num_blink_enable_mask != 0) {
        if (--lcd_num_blink_prescale == 0) {
            lcd_num_disable_mask = lcd_num_blink_enable_mask ^ lcd_num_disable_mask;
            LCD_NUM_REFRESH;
            lcd_num_blink_prescale = lcd_num_blink_prescale_value;
        }
    }
#endif
}
//! @}
/*EOF*/
