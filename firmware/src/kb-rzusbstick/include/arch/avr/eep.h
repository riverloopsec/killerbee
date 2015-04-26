// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Header file for accessing production parameters stored in EEPROM.
 *
 * \defgroup grEEP Interface to internal and external EEPROM.
 * \ingroup grHardwareInterfaceLayer
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
 * $Id: eep.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef EEP_H
#define EEP_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "eep_conf.h"

//! \addtogroup grEEP
//! @{
/*================================= MACROS           =========================*/
/*! \brief This address map is used when accessing variables stored in EEPROM 
 *        (Internal and External).
 *
 * \note The user should not use internal EEPROM addresses below EE_LAST_ADR.
 */
#define EE_MAGIC            0x42

#define EE_MAGIC_ADR        0
#define EE_MAGIC_SIZE       1

#define EE_PRODDAY_ADR      (EE_MAGIC_ADR + EE_MAGIC_SIZE)
#define EE_PRODDAY_SIZE     1

#define EE_PRODMONTH_ADR    (EE_PRODDAY_ADR + EE_PRODDAY_SIZE)
#define EE_PRODMONTH_SIZE   1

#define EE_PRODYEAR_ADR     (EE_PRODMONTH_ADR + EE_PRODMONTH_SIZE)
#define EE_PRODYEAR_SIZE    1

#define EE_MAC_ADR          (EE_PRODYEAR_ADR + EE_PRODYEAR_SIZE)
#define EE_MAC_SIZE         8

#define EE_CAPARRAY_ADR     (EE_MAC_ADR + EE_MAC_SIZE)
#define EE_CAPARRAY_SIZE    1

#define EE_1V1_CAL_VALUE_ADR   (EE_MAC_ADR + EE_MAC_SIZE)
#define EE_1V1_CAL_VALUE_SIZE  2

#define EE_HWREV_ADR        (EE_1V1_CAL_VALUE_ADR + EE_1V1_CAL_VALUE_SIZE)
#define EE_HWREV_SIZE       1

#define EE_BOOT_MAGIC_ADR (EE_HWREV_ADR + EE_HWREV_SIZE)
#define EE_BOOT_MAGIC_SIZE (1)
#define EE_BOOT_MAGIC_VALUE (53)

#define EE_LAST_ADR         (EE_BOOT_MAGIC_ADR + EE_BOOT_MAGIC_SIZE -1)
/*@}*/
/*================================= TYEPDEFS         =========================*/
/*! brief This enumeration defines the possible eeprom sources available. */
typedef enum {
    EEP_SOURCE_UNSUPPORTED = 0, //!< This the idle state for the eeprom before it has been successfully initialized.
    EEP_SOURCE_INTERNAL,        //!< Internal EEPROM is used.
    EEP_SOURCE_EXTERNAL         //!< External TWI serial EEPROM is used.
} eeprom_source_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
/*! \brief This function must called to initialize the EEPROM. External serial 
 *         EEPROM will be is preferred. Internal EEPROM is tried as last option.
 *
 *  \retval true The external or internal EEPROM has been successfully initialized.
 *  \retval false No EEPROM source could be started.
 */
bool eep_init(void);


/*! \brief This function must be called when the EEPROM interface is no longer 
 *         needed. It will deinit other modules depending on configuration.
 */
void eep_deinit(void);


/*! \brief This function returns the production day of the kit. 
 *
 *  \returns Production day (1 to 31).
 */
uint8_t eep_read_production_day(void);


/*! \brief This function returns the production month of the kit.
 *
 *  \returns Production month (1 to 12).
 */
uint8_t eep_read_production_month(void);


/*! \brief This function returns the production year of the kit.
 *
 *  \returns Production year (08 - ).
 */
uint8_t eep_read_production_year(void);


/*! \brief This function returns the IEEE address of the kit. 
 *
 *  \returns 64-bit IEEE address of the node (Unique HW tag).
 */
uint64_t eep_read_ieee_address(void);


/*! \brief This function returns the cap array settings for the radio transceiver.
 *
 *  \returns Calibration value for the radio transceiver's internal CAP array.
 */
uint8_t eep_read_cap_array(void);


/* This function returns the ??? settings for the kit. */
uint16_t eep_read_1v1(void);


/*! \brief This function returns the hw revision number of the kit.
 *
 *  \returns HW revision number.
 */
uint8_t eep_read_hw_revision(void);


/*! \brief Read random data from EEPROM.
 *
 *  \param[in] eeprom_address Start address reading from.
 *  \param[in] data_buffer_ptr Pointer to memory where the read data is to be stored.
 *  \param[in] length Length of data to read in bytes.
 *
 *  \retval true Data read successfully.
 *  \retval false Data could not be read.
 */
bool eep_read(eep_address_t eeprom_address, uint8_t *data_buffer_ptr, eep_length_t length);


/*! \brief Write random data to EEPROM.
 *
 *  \note It is not possible write to the external EEPROM.
 *
 *  \param[in] eeprom_address Start address to write to.
 *  \param[in] data_buffer_ptr Pointer to memory where the data to be written is stored.
 *  \param[in] length Length of data to be written in bytes.
 *
 *  \retval true Data written successfully.
 *  \retval false Data could not be written.
 */
bool eep_write(eep_address_t eeprom_address, const uint8_t *data_buffer_ptr, eep_length_t length);
//! @}
#endif
/*EOF*/
