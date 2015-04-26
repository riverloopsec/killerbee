// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Bootloader for the m3290p on AVR Raven
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
 * $Id: avrraven_3290p_bl.c 41821 2008-05-16 08:39:42Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "avrraven_3290p_bl.h"

#include "compiler.h"

#include "board.h"
#include "wdt_avr.h"
#include "sleep_avr.h"

#include "macros.h"
#include "bl.h"

#include "self_programming.h"
#include "self_programming_conf.h"

#include "lcd.h"
#include "led.h"

#include "adc.h"



/*========================= MACROS                   =========================*/
/*! \brief Reference number of cycles that should be counted by the internal timer
 *         running from the RC oscillator during calibration.
 */
#define RC_OSC_REFERENCE_COUNT      (F_CPU*31250UL/4000000UL)
#define RC_OSC_REFERENCE_COUNT_MAX  (1.005*F_CPU*31250UL/4000000UL)
#define RC_OSC_REFERENCE_COUNT_MIN  (0.995*F_CPU*31250UL/4000000UL)

//! Bootloader revision
#define AVRRAVEN_3290P_BL_REV        REVISION_TO_INT16(AVRRAVEN_3290P_BL_REV_MAJ, AVRRAVEN_3290P_BL_REV_MIN)

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! application program boot vector
static void(* const jmp_app_section)(void) = 0x0000;

//! Firmware data buffer
static uint8_t page_buffer[SF_PAGE_BYTE_SIZE];

//! Progmem variable holding bootloader revision
PROGMEM_LOCATION(static uint16_t ATmega3290p_bl_rev, FW_REV_ADR_M3290P_BL) = AVRRAVEN_3290P_BL_REV;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*! \brief  Jump to address 0x00000 and move interrupt vectors back to app section
 *
 */
static void app_start(void);


/*! \brief  Basic MCU initialization
 *
 * \retval true     Initialization OK
 * \retval false    Initialization FAIL
 */
static bool avr_init(void);


/*! \brief  Intern RC oscillator calibration
 *
 */
static void intosc_cal(void);


/*! \brief  Error handler for fatal errors
 *
 */
static void error_handler(void);


/*! \brief  System reboot
 *
 *              The watchdog timer is used to give a hard reset
 */
static void reboot(void);


/*! \brief  Upgrade application program
 *
 *              The application prgram is read from data flash
 *
 *  \param[in]  fw_image_adr    Data flash address to start copying the new firmware
 *
 */
static void do_fw_upgrade(df_adr_t fw_image_adr);


/*! \brief  Read current battery voltage
 *
 *
 *  \return         Current battery voltage in mV
 */
static int16_t supply_voltage_read(void);

/*========================= IMPLEMENTATION           =========================*/

#if defined(__ICCAVR__)
#pragma type_attribute = __task
void main(void) {
#else
int main(void) {
#endif
    
    uint32_t blcc;
    
    // Prevent optimizing of bootloader revision in progmem
    volatile uint16_t tmp2 = ATmega3290p_bl_rev;
    
    // Disable watchdog
    watchdog_disable();
    
    
    // Configuring LCD with Extern clock (TOSC, 32.768kHz)
    //                      32786 Hz          32786 Hz
    //  frame_rate = ------------------ = ------------- = 32 Hz
    //               8 * .prescl * .div     8 * 16 * 8
    //
    lcd_config_t lcd_config = LCD_DEFAULT_CONFIG;
    
    // Initialization
    if (true != avr_init()) {               // Generic MCU initialization
        error_handler();
    } else if (df_init() != 0) {            // Data flash initialization
        error_handler();
    } else if (lcd_init(lcd_config) != 0) { // Display
        error_handler();
    } else if (led_init() != 0) {           // Led
        error_handler();
    }
    
   
    /* start timer 2*/
    ASSR |= (1<<AS2);    // Asynchronous operation
    TCCR2A &= ~((1<<CS22)|(1<<CS21)|(1<<CS20));
    TCCR2A |= (1<<CS20);
    
    
    // Check shorting of "Factory default pins"
    uint8_t prr = PRR;
	PRR &= ~(1 << PRADC);
	DIDR1 &= ~((1 << AIN1D)|(1 << AIN0D));
	PRR = prr;
    
    BOOT_DDR |= (1 << BOOT_TX);  // Tx output
    BOOT_PORT &= ~(1 << BOOT_TX);  // Tx low
    
    BOOT_DDR &= ~(1 << BOOT_RX); // Rx input
    BOOT_PORT |= (1 << BOOT_RX); // Rx pullup on
    
    if ((BOOT_PIN & (1 << BOOT_RX)) != (1 << BOOT_RX)) { // Rx pin low?
        /* Check that RX goes high when TX is pulled high. */
        BOOT_PORT |= (1 << BOOT_TX);                            // Set Tx high
        
        nop();
        nop();
        nop();
        nop();
        
        if ((BOOT_PIN & (1 << BOOT_RX)) == (1 << BOOT_RX)) {    // Rx high?
            intvecs_to_boot(); // move interrutp vectors to boot section, and start boot loader
            sei();
            
            // Check supply voltage
            if (supply_voltage_read() < 2600) {
                lcd_puts("LOW BAT");
                error_handler();
            }
            
            BLCC_WRITE(BLCC_NORMAL_APP_START); // write communication channel in case abnormal exit of boot loader (power off etc.)
            
            lcd_symbol_set(LCD_SYMBOL_RAVEN);
            lcd_puts("WRITING");
            led_status_set(LED_FAST_BLINK);
            do_fw_upgrade(M3290P_FLASH_FD_IMG_ADR);

            // Signal ATmega3290p application program that FW upgrade is complete
            // This makes the application program continue upgrade ATmega1284p after booting
            BLCC_WRITE(BLCC_LOAD_FACTORY_DEFAULT);
            
            app_start();    // start application program
        }
    }
    
    // Read bootloader communication channel in EEPROM and take proper action
    BLCC_READ(blcc);
    if (blcc == BLCC_FW_UPGRADE_START_REQUEST_FROM_APP) {
        intvecs_to_boot(); // move interrutp vectors to boot section, and start boot loader
        sei();
        
        // Check supply voltage
        if (supply_voltage_read() < 2600) {
            lcd_puts("LOW BAT");
            error_handler();
        }
        
        BLCC_WRITE(BLCC_NORMAL_APP_START); // write communication channel in case abnormal exit of boot loader (power off etc.)
        
        lcd_symbol_set(LCD_SYMBOL_RAVEN);
        lcd_puts("WRITING");
        led_status_set(LED_FAST_BLINK);
        do_fw_upgrade(M3290P_FLASH_USR_IMG_ADR);
        
        // Signal ATmega3290p application program that FW upgrade is complete
        BLCC_WRITE(BLCC_FW_UPGRADE_COMPLETE);
        reboot();
    } else if (blcc == BLCC_FW_UPGRADE_COMPLETE) {
        BLCC_WRITE(BLCC_FW_UPGRADE_COMPLETE);
        sei();
        app_start();    // start application program
    } else if (blcc == BLCC_RESTART_REQUEST_FROM_APP) {
        /* Start application program*/
        BLCC_WRITE(BLCC_NORMAL_APP_START);
        sei();
        app_start();
    } else {
        /*else, start application program*/
        BLCC_WRITE(BLCC_NORMAL_APP_START);
        sei();
        app_start();
    }
}

static int16_t supply_voltage_read(void)
{
    int16_t adc_res;
    // Read band gap voltage with Avcc as ref. Use result to calulate Vcc
	adc_init(ADC_CHAN_VBG, ADC_TRIG_FREE_RUN, ADC_REF_AVCC, ADC_PS_128);
    
    // wait for band gap voltage to stabilize
    delay_us(150);
    
    // read ADC
    adc_conversion_start();
    while ((adc_res = adc_result_get(ADC_ADJ_RIGHT)) == EOF ){;}
    adc_deinit();
    
    // Return battery voltage in mV
    return (int16_t)((1100L*1024L)/(long)adc_res);
}

static void do_fw_upgrade(df_adr_t fw_image_adr)
{
    // Write FW image from data flash to ATmega3290p application progam area
    for (sp_adr_t adr = 0; adr < SF_APPLICATION_END; adr += SF_PAGE_BYTE_SIZE) {
        // Read data from flash
        df_read(fw_image_adr + adr, SF_PAGE_BYTE_SIZE, page_buffer);
        
        // Display current page
        int page = adr/SF_PAGE_BYTE_SIZE + 1;
        lcd_num_putdec(page, LCD_NUM_PADDING_SPACE);
    
        // Write page to ATmega3290 Application section
        sp_write_flash_page(adr, page_buffer);
    }
}

static void app_start(void)
{
    cli();
    PRR = 0x00;  
    DIDR0 = 0x00;  
    DIDR1 = 0x00;        
    intvecs_to_app();
    jmp_app_section();
    /*program execution continues in application progam*/
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
    
    avr_set_sleep_mode(SLEEP_IDLE);
    
    /* System is started with the CLKPR set to divide by eight. To run at 4 MHz
     * the value of the CLKPR must be updated.
     */
    ENTER_CRITICAL_REGION();
    CLKPR = (1<<CLKPCE);
    CLKPR = ((0<<CLKPS3)|(0<<CLKPS2)|(0<<CLKPS1)|(1<<CLKPS0));
    LEAVE_CRITICAL_REGION();
    
    intosc_cal();

    return true;
}

static void reboot(void)
{
    watchdog_timeout_set(WDT_TIMEOUT_16K);
    watchdog_enable();
    while(1){
        ;
    }
}

static void error_handler(void)
{
    while(1){
        ;
    }
}

static void intosc_cal(void)
{
    /* Calibrate RC Oscillator: The calibration routine is done by clocking TIMER2
     * from the external 32kHz crystal while running an internal timer simultaneously.
     * The internal timer will be clocked at the same speed as the internal RC 
     * oscillator, while TIMER2 is running at 32768 Hz. This way it is not necessary
     * to use a timed loop, and keep track cycles in timed loop vs. optimization 
     * and compiler.
     */
    uint8_t osccal_original = OSCCAL;
    
    PRR &= ~(1 << PRTIM1); // Enable Timer 1
    
    TIMSK2 = 0x00; // Disable Timer/Counter 2 interrupts.
	TIMSK1 = 0x00; // Disable Timer/Counter 1 interrupts.
    
    
    /* Enable TIMER/COUNTER 2 to be clocked from the external 32kHz clock crystal. 
     * Then wait for the timer to become stable before doing any calibration.
     */
    ASSR |= (1 << AS2);     
    while (ASSR & ((1<<TCN2UB)|(1<<OCR2UB)|(1<<TCR2UB))) { ; }
    
    uint8_t counter = 128;
    bool cal_ok = false;
    
    
    do {
        
        TCNT2 = 0;
        TCNT1 = 0;
        
        TCCR2A = 1 << CS20; 
        TCCR1B = 1 << CS10;
        
        TIFR2 = 0xFF;

        /* Wait for TIMER/COUNTER 2 to overflow. Stop TIMER/COUNTER 1 and 2, and
         * read the counter value of TIMER/COUNTER 1. It will now contain the 
         * number of cpu cycles elapsed within the period.
         */
        while ((TIFR2 & (1 << TOV2)) != (1 << TOV2)) { ; }
        
        TCCR2A = 0; 
        TCCR1B = 0;
        
        uint16_t tmp = TCNT1;
        
        /* Simple control loop. */
        if (tmp < RC_OSC_REFERENCE_COUNT_MIN) {
            /* Too slow. Put the hammer down. */
            OSCCAL++;
        } else if (tmp > RC_OSC_REFERENCE_COUNT_MAX) {
            /* Too fast, retard. */
            OSCCAL--;
        } else {
            /* The CPU clock frequency is now within +/- 0.5% of the target value. */
            cal_ok = true;
        } // END: if (temp < (uint16_t)(0.995 * RC_OSC_REFERENCE_COUNT)) ...
        counter--;
    } while ((counter != 0) && (false == cal_ok));
    
    if (true != cal_ok) {
        /* We failed, therefore restore previous OSCCAL value. */
        OSCCAL = osccal_original;
    } // END: if (true != cal_ok) ...
    
    ASSR &= ~(1 << AS2);
    
    /* Disable both timers again to save power. */
    PRR |= (1 << PRTIM1); // Turn off Timer/Counter 1 module.
    TCCR2A = 0x00;
    TIMSK2 = 0x00;
}

/*EOF*/
