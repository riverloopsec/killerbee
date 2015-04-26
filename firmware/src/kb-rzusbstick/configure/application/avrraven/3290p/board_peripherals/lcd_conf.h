// This file has been prepared for Doxygen automatic documentation generation.
/*!\file *********************************************************************
 *
 *\brief  Configuration file for the AVR Raven LCD driver
 *
 *\par Application note:
 *      AVR2017: RZRAVEN FW
 *
 *\par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 *\author
 *      Atmel Corporation: http://www.atmel.com\n
 *      Support email: avr@atmel.com
 *
 * $Id: lcd_conf.h 41558 2008-05-13 12:10:19Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __LCD_CONF_H__
#define __LCD_CONF_H__

/*========================= INCLUDES                 =========================*/
#include "compiler.h"

#ifdef AVRRAVEN_3290P
#include "vrt_kernel.h"

#else
#error "LCD configuration not supported."
#endif

//!\addtogroup grLCDLowLevel
//! @{
/*========================= MACROS                   =========================*/
#ifdef AVRRAVEN_3290P

#ifndef LCD_DRIVER_MINIMAL

//! Macro for handling LCD events. This implementation is based on the vrt_kernel
#define LCD_POST_EVENT(evt) do {\
    vrt_post_event(evt, (uint8_t*)NULL);\
} while(0)

extern void int_evt_lcd_shift_left(void* evt);
extern void int_evt_lcd_cursor_toggle(void* evt);
extern void int_evt_lcd_num_refresh(void* evt);
#define LCD_SHIFT_LEFT      vrt_post_event(int_evt_lcd_shift_left, (void*)NULL);
#define LCD_CURSOR_TOGGLE   vrt_post_event(int_evt_lcd_cursor_toggle, (void*)NULL);
#define LCD_NUM_REFRESH     vrt_post_event(int_evt_lcd_num_refresh, (void*)NULL);
    

#else


//! Macro for handling LCD events. This implementation uses callbacks in the ISR
#define LCD_POST_EVENT(evt) do {\
    lcd_event(evt);\
} while(0)

#define LCD_SHIFT_LEFT      lcd_text_shift_left()
#define LCD_CURSOR_TOGGLE   lcd_cursor_toggle()
#define LCD_NUM_REFRESH     lcd_num_refresh()

#endif


#else
#error "LCD configuration not supported."
#endif

//! Default LCD configuration
#define LCD_DEFAULT_CONFIG\
	{{{\
		LCD_BLANKING_OFF,\
		LCD_BUFFER_ON,\
        LCD_WAVE_LOW_POWER\
	}},{{\
		LCD_BIAS_HALF,\
		LCD_CLOCK_EXTERN,\
	}},{{\
		LCD_DIV_8,\
		LCD_PRESCL_16\
	}},{{\
		LCD_CONTRAST_2_75,\
		LCD_DRIVE_FULL\
	}},\
        LCD_SCROLLING_MEDIUM\
    };

/* // Better version (Not gcc compatible)
    lcd_config_t lcd_config = {
        .blanking   = LCD_BLANKING_OFF,
        .buffer     = LCD_BUFFER_ON,
        .waave      = LCD_WAVE_LOW_POWER,
        .bias       = LCD_BIAS_HALF,
        .clock      = LCD_CLOCK_EXTERN,
        .div        = LCD_DIV_8,
        .prescl     = LCD_PRESCL_16,
    //  .contrast   = LCD_CONTRAST_2_75 // <<< set from user config later
        .drive      = LCD_DRIVE_FULL
	};
*/
/*========================= TYPEDEFS                 =========================*/
/*========================= PUBLIC VARIABLES         =========================*/
/*========================= PUBLIC FUNCTIONS         =========================*/
//! @}
#endif // __LCD_CONF_H__
/*EOF*/
