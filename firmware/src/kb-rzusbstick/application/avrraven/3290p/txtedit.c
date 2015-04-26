// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Text editor for generic use on the AVR Raven
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
 * $Id: txtedit.c 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "vrt_kernel.h"

#include "lcd.h"
#include "txtedit.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
/// Variable holding size of current text (char count befor zero terminanion)
static int8_t txtedit_size;

/// Buffer pointer. == NULL when not initialized
static uint8_t* txtedit_buffer = NULL;

/// Pointer to text buffer currently being edited
static int8_t txtedit_buffer_ptr;

/// Size of text buffer currently being edited
static int8_t txtedit_buffer_size;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int txtedit_init(uint8_t* txtbuffer, uint8_t txtbuffer_size)
{
    // Return EOF if txtedit_done() was not called after previous txtedit_init()
    if (txtedit_buffer != NULL) {
        return EOF;
    }
    
    // Return EOF if buffer size is less than 2 (at least one char + zero termination must fit)
    if (txtbuffer_size < 2) {
        return EOF;
    }
    
    // Initialize global variables 
    txtedit_buffer_size = txtbuffer_size;
    txtedit_buffer = txtbuffer;
    txtedit_buffer_ptr = 0;
    txtedit_size = 0;
    
    // Look for alphanumeric chars and zero termination in buffer
    for (int i = 0; (txtedit_buffer[i] != '\0') && (i < txtbuffer_size); ++i) {
        if ((isalnum((int)txtedit_buffer[i])) || (txtedit_buffer[i] == ' ')) { // if an alphanumeric character or space is found, increase text size
            txtedit_size++;
        } else {                        // if not, zero terminate buffer and exit
            txtedit_buffer[i] = '\0';
            break;
        }
    }
    
    // If no text in buffer, fill in a space and zero terminate
    if (txtedit_size == 0) {
        txtedit_buffer[0] = ' ';
        txtedit_buffer[1] = '\0';
        txtedit_size = 1;
    }
    
    // If buffer not zero terminated, over write last char with '\0'
    if (txtedit_size == txtbuffer_size) {
        txtedit_buffer[--txtedit_size] = '\0';
    }
    
    // Trim trailing spaces and update text size
    for (int i = txtedit_size-1; i > 0; i--) {
        if (txtedit_buffer[i] == ' ') {
            txtedit_buffer[--txtedit_size] = '\0';
        } else {
            break;
        }
    }
    
    // Display initial buffer text
    int offset = (txtedit_buffer_ptr>=LCD_ALPHA_DIGIT_COUNT) ? (txtedit_buffer_ptr - LCD_ALPHA_DIGIT_COUNT + 1) : 0;
    int length = ((txtedit_size-offset)>LCD_ALPHA_DIGIT_COUNT) ? LCD_ALPHA_DIGIT_COUNT : (txtedit_size-offset);
    int tmp = txtedit_buffer_size - txtedit_size - 1;
    lcd_puta(&txtedit_buffer[offset], length);
    lcd_num_putdec(tmp, LCD_NUM_PADDING_SPACE);
    
    // Set cursor position
    lcd_cursor_set(txtedit_buffer_ptr, LCD_CURSOR_START_OFF);
    
    return 0;
}

int8_t txtedit_size_get(void)
{
    return txtedit_size;
}

void txtedit_event(txtedit_evt_t event)
{
    bool cursor_start_state = LCD_CURSOR_START_OFF;
    
    // Convert to upper case if lower case letter found
    if (islower((int)txtedit_buffer[txtedit_buffer_ptr])) {
        int ch = toupper((int)txtedit_buffer[txtedit_buffer_ptr]);
        txtedit_buffer[txtedit_buffer_ptr] = ch;
    }
    
    // Check event, and take proper action
    switch (event) {
    case TXTEDIT_KEY_UP: // Get next printable character from ASCII table
        if (txtedit_buffer[txtedit_buffer_ptr] == '9') {
            txtedit_buffer[txtedit_buffer_ptr] = ' ';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '-') {
            txtedit_buffer[txtedit_buffer_ptr] = '/';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '+') {
            txtedit_buffer[txtedit_buffer_ptr] = '-';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '\x40') { // @
            txtedit_buffer[txtedit_buffer_ptr] = '*';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '<') {
            txtedit_buffer[txtedit_buffer_ptr] = '>';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '_') {
            txtedit_buffer[txtedit_buffer_ptr] = '<';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == ' ') {
            txtedit_buffer[txtedit_buffer_ptr] = 'A';
        } else {
            (txtedit_buffer[txtedit_buffer_ptr])++;
        }
        break;
    case TXTEDIT_KEY_DOWN: // Get previous printable character from ASCII table
        if (txtedit_buffer[txtedit_buffer_ptr] == ' ') {
            txtedit_buffer[txtedit_buffer_ptr] = '9';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '/'){
            txtedit_buffer[txtedit_buffer_ptr] = '-';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '-'){
            txtedit_buffer[txtedit_buffer_ptr] = '+';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '*'){
            txtedit_buffer[txtedit_buffer_ptr] = '\x40'; // @
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '>') {
            txtedit_buffer[txtedit_buffer_ptr] = '<';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == '<') {
            txtedit_buffer[txtedit_buffer_ptr] = '_';
        } else if (txtedit_buffer[txtedit_buffer_ptr] == 'A'){
            txtedit_buffer[txtedit_buffer_ptr] = ' ';
        } else {
            (txtedit_buffer[txtedit_buffer_ptr])--;
        }
        break;
    case TXTEDIT_KEY_LEFT:
        // Move active position left if possible
        if (txtedit_buffer_ptr>0) {
            txtedit_buffer_ptr--;
        }
        // Start with cursor ON
        cursor_start_state = LCD_CURSOR_START_ON;
        break;
    case TXTEDIT_KEY_RIGHT:
        if (txtedit_buffer_ptr < (txtedit_buffer_size-2)) {
            // Move active position right if possible
            if (++txtedit_buffer_ptr == txtedit_size) {
                txtedit_buffer[txtedit_buffer_ptr] = ' ';
                
                // Put new zero termination at end of buffer
                txtedit_buffer[++txtedit_size] = '\0';
            }
        }
        // Start with cursor ON
        cursor_start_state = LCD_CURSOR_START_ON;
        break;
    case TXTEDIT_KEY_ENTER:
        // Trim trailing spaces and update text size
        for (int i = txtedit_size-1; i > 0; i--) {
            if (txtedit_buffer[i] == ' ') {
                txtedit_buffer[--txtedit_size] = '\0';
            } else {
                break;
            }
        }
        break;
    default:
        // Event not defined, do nothing
        break;
    }
    
    // Trim trailing spaces (after cursor) and update text size
    for (int i = txtedit_size-1; i > txtedit_buffer_ptr; i--) {
        if (txtedit_buffer[i] == ' ') {
            txtedit_buffer[--txtedit_size] = '\0';
        } else {
            break;
        }
    }
    
    // Print text to display
    int8_t offset = (txtedit_buffer_ptr>=LCD_ALPHA_DIGIT_COUNT) ? (txtedit_buffer_ptr - LCD_ALPHA_DIGIT_COUNT + 1) : 0;
    int length = ((txtedit_size-offset)>LCD_ALPHA_DIGIT_COUNT) ? LCD_ALPHA_DIGIT_COUNT : (txtedit_size-offset);
    int tmp = txtedit_buffer_size - txtedit_size - 1;
    lcd_puta(&txtedit_buffer[offset], length);
    lcd_num_putdec(tmp, LCD_NUM_PADDING_SPACE);
    
    // Update cursor
    int8_t cursor_pos = txtedit_buffer_ptr - offset;
    lcd_cursor_set(cursor_pos, cursor_start_state);
}

void txtedit_done(void)
{
    // Display edited text 
    lcd_puts((const char*)txtedit_buffer);
    
    // Clear buffer size left from numeric digits
    lcd_num_clr_all();
    
    // Clear cursor
    lcd_cursor_clr();
    
    // Reset internal variables
    txtedit_buffer = 0;
    txtedit_buffer_ptr = 0;
    txtedit_buffer_size = 0;
}
//! @}
/*EOF*/
