// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the main loop for the AVR Raven application program
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
 * $Id: avrraven_3290p.c 41767 2008-05-15 18:53:24Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/


/*================================= INCLUDES         =========================*/

// 
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "wdt_avr.h"

// Peripherals
#include "key.h"
#include "led.h"
#include "mic.h"
#include "temp.h"
#include "lcd.h"
#include "board.h"

// Modules
#include "mbox.h"
#include "battery.h"
#include "sipc.h"
#include "sleep_avr.h"
#include "tick.h"
#include "asy_tmr.h"
#include "audio.h"

// Kernel
#include "vrt_kernel.h"
#include "vrt_mem.h"

// Utilities
#include "sfs.h"
#include "self_programming.h"

// Misc.
#include "timndate.h"
#include "rvn_loc.h"
#include "cmd_if_3290p.h"
#include "macros.h"
#include "loc.h"
#include "int_evt.h"
#include "mmap.h"
#include "error.h"

#include "math.h"


/*========================= MACROS                   =========================*/
//! Number of memory partitions
#define MEMORY_PARTITIONS   (3)


//@{! Memory block sizes
#define MEMORY_BLOCKSIZE0 (32)
#define MEMORY_BLOCKSIZE1 (64)
#define MEMORY_BLOCKSIZE2 (128)
//@}


//@{! Memory block counts
#define MEMORY_BLOCKCOUNT0 (2)
#define MEMORY_BLOCKCOUNT1 (2)
#define MEMORY_BLOCKCOUNT2 (2)
//@}

//! Current application fw revision
#define AVRRAVEN_3290P_APP_REV      REVISION_TO_INT16(AVRRAVEN_3290P_APP_REV_MAJ, AVRRAVEN_3290P_APP_REV_MIN)


/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/
avrraven_t avrraven;

/*========================= PRIVATE VARIABLES        =========================*/
//@{! Memory partitions
static vrt_mem_partition_desc_t desc0;
static vrt_mem_partition_desc_t desc1;
static vrt_mem_partition_desc_t desc2;
//@}

//@{! Statically allocated memory blocks
static uint8_t partition0[MEMORY_BLOCKSIZE0 * MEMORY_BLOCKCOUNT0];
static uint8_t partition1[MEMORY_BLOCKSIZE1 * MEMORY_BLOCKCOUNT1];
static uint8_t partition2[MEMORY_BLOCKSIZE2 * MEMORY_BLOCKCOUNT2];
//@}

//! Progmem variable holding application program revision
PROGMEM_LOCATION(static uint16_t ATmega3290p_appl_fw_rev, FW_REV_ADR_M3290P_APP) = AVRRAVEN_3290P_APP_REV;

//! Defined in the vtr_kernel.c. Access needed to test for pending events before going to sleep
extern vrt_event_queue_size_t vrt_event_fifo_items_used;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*! \brief  General MCU initialization
 *
 * \retval true     Initialization OK
 * \retval false    Initialization FAIL
 */
static bool avr_init(void);


/*! \brief Thid function evaluates if the VRT system is in idle state. That is
 *         no tasks running.
 *
 *  \retval true The VRT framework is ready to SLEEP.
 *  \retval false The VRT framework is not ready to SLEEP.
 */
static bool vrt_in_idle(void);


/*! \brief  Initialize memory manager
 */
static int memory_init(void);


/*! \brief  Enter sleep mode
 */
static void go_to_sleep(void);


/*! \brief  Error handler for fatal errors
 *
 */

/*========================= IMPLEMENTATION           =========================*/
#if defined(__ICCAVR__)
#pragma type_attribute = __task
void main(void) {
#else
int main(void) {
#endif
    
    // Prevent optimizing of application revision in progmem
    volatile uint16_t tmp = ATmega3290p_appl_fw_rev;
    
    
    // System initialization
    if (true != avr_init()) {               // Generic MCU initialization
        ERROR();
    } else if (true != vrt_init()) {        // Kernel init
        ERROR();
    } else if (memory_init() != 0) {        // Initialize memory manager.
        ERROR();
    } else if (true != cmd_if_init()) {     // Command interface (does also "sipc_init()")
        ERROR();
    } else if (tick_init() != 0) {          // System tick
        ERROR();
    }
    
    
    // Load user configuration from EEPROM
    USER_CONFIG_LOAD();
    
    
    // Test if basic user configuration is valid. If not, load with default values
    if (!USER_CONFIG_BASIC_VALID()) {
        avrraven.user_config.basic_valid             = USER_CONFIG_BASIC_VALID_ID;
        avrraven.user_config.panid                   = 0xBABE;
        avrraven.user_config.channel                 = 11;

        USER_CONFIG_STORE();
    }
    
    
    // Test if extended user configuration is valid. If not, load with default values
    // and indicate data flash content not valid. Setting this flag to "false" results
    // in a formatting later in the initialization. All users data will be lost!
    if (!USER_CONFIG_EXTENDED_VALID()) {
        avrraven.user_config.extended_valid          = USER_CONFIG_EXTENDED_VALID_ID;
        avrraven.user_config.unit.temp               = TEMP_UNIT_CELCIUS;
        avrraven.user_config.unit.clock              = TIME_CLOCK_UNIT_24;
        avrraven.user_config.txtmsg_preview          = true;
        avrraven.user_config.fw_upgr_auto            = true;
        avrraven.user_config.fw_rcv_auto             = true;
        avrraven.user_config.join_auto               = true;
        avrraven.user_config.show_hidden_files       = true;
        avrraven.user_config.show_system_files       = true;
        avrraven.user_config.debug_enable            = false;
        avrraven.user_config.data_flash_valid        = false;
        avrraven.user_config.lcd.contrast            = LCD_CONTRAST_2_75;
        avrraven.user_config.lcd.scrolling           = LCD_SCROLLING_MEDIUM;
        strncpy_P((char*)avrraven.user_config.name,  AVRRAVEN_DEFAULT_NAME, AVRRAVEN_DEFAULT_NAME_SIZE);
        
        USER_CONFIG_STORE();
    }
    
    
    // Load default configuration for LCD driver and read constrast level from user variable
    lcd_config_t lcd_config = LCD_DEFAULT_CONFIG;
    lcd_config.contrast = avrraven.user_config.lcd.contrast;
    lcd_config.scrolling = avrraven.user_config.lcd.scrolling;

    
    // Driver initialization
    if (asy_tmr_init() != 0) {              // Asynchronous timer
        ERROR();
    } else if (key_init() != 0) {           // Joystick driver
        ERROR();
    } else if (led_init() != 0) {           // Raven eye LED driver
        ERROR();
    } else if (lcd_init(lcd_config) != 0) { // Display driver
        ERROR();
    } else if (battery_init() != 0) {       // Battery icon driver
        ERROR();
    } else if (temp_init() != 0) {          // Temperature sensor driver
        ERROR();
    } else if (timndate_init() != 0) {      // Clock driver
        ERROR();
    } else if (audio_init() != 0) {         // Audio playback driver (does also "speaker_init()")
        ERROR();
    }
    
    
    // Read battery voltage before continue initialization
    int_evt_update_batt((void*)NULL);
    
    
    // These modules uses the data flash and thus depends on stable Vcc
    if (avrraven.status.batt_low == false) {
        if(sfs_init() != 0) {                           // File system (does also "df_init()")
            ERROR();
        } else if (avrraven.user_config.data_flash_valid == false) {// format drive, and set valid flag if content not valid
            lcd_puts_P("WAIT");
            sfs_format_drive();
            avrraven.user_config.data_flash_valid = true;
            USER_CONFIG_STORE();
        }
        
        if (mbox_init() != 0) {                          // Mail box
            ERROR();
        }
    }
    
    
    // Disable the debug interface if disabled in user configuration
    if (avrraven.user_config.debug_enable == false) {
        // Disable the JTAG interface
        MCUCR |= (1<<JTD);
        MCUCR |= (1<<JTD);
    }
    
    
    // Set initial state
    avrraven.status.state = STATE_STARTUP;
    
    
    // Read current application revision stored in progmem
    avrraven.status.m3290p_app_rev = sp_read_flash(FW_REV_ADR_M3290P_APP);
    avrraven.status.m3290p_app_rev |= sp_read_flash(FW_REV_ADR_M3290P_APP + 1) << 8;
    
    
    // Read current bootloader revision stored in progmem
    avrraven.status.m3290p_bl_rev = sp_read_flash(FW_REV_ADR_M3290P_BL);
    avrraven.status.m3290p_bl_rev |= sp_read_flash(FW_REV_ADR_M3290P_BL + 1) << 8;
    
    
    // Enable interrupts
    sei();
    
    
    // Register periodic events
    asy_tmr_get(int_evt_check_key, (void*)NULL, 50);
    asy_tmr_get(int_evt_update_batt, (void*)NULL, 1000);
    
    
    // Post system startup event
    vrt_post_event(int_evt_startup, (void*)NULL);
    
    
    // Main loop, 
    while (true) {
        vrt_dispatch_event();
    
        // Serve asynchronous timer
        asy_tmr_task(tick_count_get());
        
        if (true == vrt_in_idle()) {
            go_to_sleep();
        }
    }
}

static bool avr_init(void) {
    /* The input buffers must be disabled before the ADC, because the disabling
     * is handled by the ADC module.
     */
	DIDR0 = (1 << ADC7D) | // Disable digital ionput buffer for analog input pins.
	        (1 << ADC6D) |
	        (1 << ADC5D) |
	        (1 << ADC4D) |
	        (1 << ADC3D) |
	        (1 << ADC2D) |
	        (1 << ADC1D) |
	        (1 << ADC0D);
	DIDR1 = (1 << AIN1D) |
	        (1 << AIN0D);

    // NOTE: Modules are turned on/off by driver.
	PRR = (1 << PRLCD)    |
	      (1 << PRTIM1)   |
	      (1 << PRSPI)    |
          (1 << PRUSART0) |
	      (1 << PRADC);

	ACSR |= (1 << ACD); // Disable Analog Comparator.

    /* System is started with the CLKPR set to divide by eight. To run at 4 MHz
     * the value of the CLKPR must be updated.
     */
    ENTER_CRITICAL_REGION();
    CLKPR = (1<<CLKPCE);
    CLKPR = ((0<<CLKPS3)|(0<<CLKPS2)|(0<<CLKPS1)|(1<<CLKPS0));
    LEAVE_CRITICAL_REGION();
    
    // Set default idle sleep mode
    SMCR = (0<<SM2)|(1<<SM1)|(0<<SM0);

    return true;
}


static bool vrt_in_idle(void) {
    
    bool idle = false;
    if (0 != VRT_GET_ITEMS_USED()) {
    } else {
        idle = true;
    }
    
    return idle;
}


static int memory_init(void)
{
    (bool)vrt_mem_add_partition(partition0, MEMORY_BLOCKSIZE0, MEMORY_BLOCKCOUNT0, &desc0);
    (bool)vrt_mem_add_partition(partition1, MEMORY_BLOCKSIZE1, MEMORY_BLOCKCOUNT1, &desc1);
    (bool)vrt_mem_add_partition(partition2, MEMORY_BLOCKSIZE2, MEMORY_BLOCKCOUNT2, &desc2);
    
    return 0;
}


void error_handler(const char* str, int num) {
    // Print information on display
    lcd_num_putdec(num, LCD_NUM_PADDING_SPACE);
    lcd_puts(str);
    
    // Enable interrupts
    ENTER_CRITICAL_REGION();
    sei();
    
    // Display raven
    lcd_symbol_set(LCD_SYMBOL_RAVEN);
    
    // Set fast blink
    led_status_set(LED_FAST_BLINK);
    
    /* Loop while checkin the joystick center button
     * If holding in for "ERROR_CONTINUE_TIMEOUT" number of interations
     * the program will continue execution
     * If not, the loop will time out and system will reboot
     */
    signed long continue_timeout = ERROR_CONTINUE_TIMEOUT;
    signed long reboot_timeout = ERROR_REBOOT_TIMEOUT;
    while (true) {
        // Enter pushed
        if (!(ENTER_PORT & (1<<ENTER_PIN))) {
            if (!continue_timeout--) {
                // Reset display status
                led_status_set(LED_OFF);
                lcd_num_clr_all();
                
                // Wait for user to release joystick
                while (!(ENTER_PORT & (1<<ENTER_PIN))) {;}
                
                // Restore interrupt state
                LEAVE_CRITICAL_REGION();
                
                // Continue program execution (Dangerous!!)
                break;
            }
        // If not, reset joystick center timeout counter
        } else {
            continue_timeout = ERROR_CONTINUE_TIMEOUT;
        }
        
        // Reboot on timeout
        if (!reboot_timeout--) {
            reboot();
        }
    }
}


static void go_to_sleep(void)
{
    avr_set_sleep_mode(SLEEP_IDLE);
    avr_sleep_enable();
    avr_mcu_sleep();
}


void reboot(void)
{
    watchdog_timeout_set(WDT_TIMEOUT_16K);
    watchdog_enable();
    while(1){
        ;
    }
}
/*EOF*/
