// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Configuration of the self programming module.
 *
 *         This file must be altered to suit the particular AVR microcontroller
 *         at hand:
 *            - SF_BOOT_PAGE_SIZE
 *            - SF_APPLICATION_END
 *            - SF_BOOTLOADER_START
 *            - SF_EEPROM_SIZE
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
 * $Id: self_programming_conf.h 41576 2008-05-13 14:10:44Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef SELF_PROGRAMMING_CONF_H
#define SELF_PROGRAMMING_CONF_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
/*================================= MACROS           =========================*/
/** \addtogroup apiSelfProgramming */
/*\@{*/

#if defined(__AT90USB1287__) || defined(__AVR_AT90USB1287__)
#define SF_PAGE_BYTE_SIZE       (256) //! Size of page in bytes.
#define SF_PAGE_WORD_SIZE       (128) //! Size of page in words.
#define SF_APPLICATION_END      (0x1DFFE) //!< End of application section in bytes.
#define SF_BOOTLOADER_START     (0x1E000) //!< Start of bootloader section in bytes.
#define SF_EEPROM_SIZE          (4096) //!< EEPROM size in bytes.


#elif defined(__ATmega1284P__) || defined(__AVR_ATmega1284P__)
#define SF_PAGE_BYTE_SIZE       (256) //! Size of page in bytes.
#define SF_PAGE_WORD_SIZE       (128) //! Size of page in words.
#define SF_APPLICATION_END      (0x1DFFE) //!< End of application section in bytes.
#define SF_BOOTLOADER_START     (0x1E000) //!< Start of bootloader section in bytes.
#define SF_EEPROM_SIZE          (4096) //!< EEPROM size in bytes.


#elif defined(__ATmega3290P__) || defined(__AVR_ATmega3290P__)
#define SF_PAGE_BYTE_SIZE       (128) //! Size of page in bytes.
#define SF_PAGE_WORD_SIZE       (64) //! Size of page in words.
#define SF_APPLICATION_END      (0x6FFE) //!< End of application section in bytes.
#define SF_BOOTLOADER_START     (0x7000) //!< Start of bootloader section in bytes.
#define SF_EEPROM_SIZE          (1024) //!< EEPROM size in bytes.


#else
#error "Unsupported device."
#endif

/*\@}*/



#if defined(__ICCAVR__)



#include <intrinsics.h>
#include <pgmspace.h>

#ifndef __HAS_ELPM__
#define _MEMATTR  __flash
#else /* __HAS_ELPM__ */
#define _MEMATTR  __farflash
#endif /* __HAS_ELPM__ */


/* IAR Embedded Workbench */
#include <inavr.h>
#define SF_GET_LOCK_BITS() __AddrToZByteToSPMCR_LPM((void __flash *)0x0001, 0x09)
#define SF_GET_LOW_FUSES() __AddrToZByteToSPMCR_LPM((void __flash *)0x0000, 0x09)
#define SF_GET_HIGH_FUSES() __AddrToZByteToSPMCR_LPM((void __flash *)0x0003, 0x09)
#define SF_GET_EXTENDED_FUSES() __AddrToZByteToSPMCR_LPM((void __flash *)0x0002, 0x09)
#define SF_GET_SIGNATURE_LOW() __AddrToZByteToSPMCR_LPM((void __flash *)0x0000, ((1 << SPMEN) | (1 << SIGRD)))
#define SF_GET_SIGNATURE_MID() __AddrToZByteToSPMCR_LPM((void __flash *)0x0002, ((1 << SPMEN) | (1 << SIGRD)))
#define SF_GET_SIGNATURE_HIGH() __AddrToZByteToSPMCR_LPM((void __flash *)0x0004, ((1 << SPMEN) | (1 << SIGRD)))
#define SF_GET_OSCCAL() __AddrToZByteToSPMCR_LPM((void __flash *)0x0001, ((1 << SPMEN) | (1 << SIGRD)))
#define SF_SET_LOCK_BITS(data) __DataToR0ByteToSPMCR_SPM(data, 0x09)
#define SF_ENABLE_RWW_SECTION() __DataToR0ByteToSPMCR_SPM(0x00, 0x11)
#define SF_WAIT_FOR_SPM() while(SPMCR_REG & (1<<SPMEN)){;};



#ifndef __HAS_ELPM__



#define SF_LOAD_PROGRAM_MEMORY(addr) __load_program_memory( (const unsigned char _MEMATTR *) (addr) )
#define SF_FILL_TEMP_WORD(addr,data) __AddrToZWordToR1R0ByteToSPMCR_SPM( (void _MEMATTR *) (addr), (unsigned short)data, 0x01 )
#define SF_PAGE_ERASE(addr) __AddrToZByteToSPMCR_SPM( (void _MEMATTR *) (addr), 0x03 )
#define SF_PAGE_WRITE(addr) __AddrToZByteToSPMCR_SPM( (void _MEMATTR *) (addr), 0x05 )



#else /* LARGE_MEMORY */



#define SF_LOAD_PROGRAM_MEMORY(addr) __extended_load_program_memory( (const unsigned char _MEMATTR *) (addr) )
#define SF_FILL_TEMP_WORD(addr,data) __AddrToZ24WordToR1R0ByteToSPMCR_SPM( (void _MEMATTR *) (addr), (unsigned short)data, 0x01 )
#define SF_PAGE_ERASE(addr) __AddrToZ24ByteToSPMCR_SPM( (void _MEMATTR *) (addr), 0x03 )
#define SF_PAGE_WRITE(addr) __AddrToZ24ByteToSPMCR_SPM( (void _MEMATTR *) (addr), 0x05 )
#endif /* LARGE_MEMORY */



#elif defined(__GNUC__)



/* AVR-GCC/avr-libc */
#include <avr/boot.h>
#include <avr/pgmspace.h>



#define SF_GET_LOCK_BITS() boot_lock_fuse_bits_get(GET_LOCK_BITS)
#define SF_GET_LOW_FUSES() boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS)
#define SF_GET_HIGH_FUSES() boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS)
#define SF_GET_EXTENDED_FUSES() boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS)

#define SF_GET_SIGNATURE_LOW() 0xFF
#define SF_GET_SIGNATURE_MID() 0xFF
#define SF_GET_SIGNATURE_HIGH() 0xFF
#define SF_GET_OSCCAL() 0xFF


#define SF_SET_LOCK_BITS(data) boot_lock_bits_set(~data)
#define SF_ENABLE_RWW_SECTION() boot_rww_enable()
#define SF_WAIT_FOR_SPM() boot_spm_busy_wait()



#ifndef LARGE_MEMORY



#define SF_LOAD_PROGRAM_MEMORY(addr) pgm_read_byte_near(addr)



#else /* LARGE_MEMORY */



#define SF_LOAD_PROGRAM_MEMORY(addr) pgm_read_byte_far(addr)



#endif /* LARGE_MEMORY */



#define SF_FILL_TEMP_WORD(addr,data) boot_page_fill(addr, data)
#define SF_PAGE_ERASE(addr) boot_page_erase(addr)
#define SF_PAGE_WRITE(addr) boot_page_write(addr)



#else
#error "Don't know your compiler."
#endif



#if defined(__AT90USB1287__) || defined(__AVR_AT90USB1287__)



#define SPMCR_REG SPMCSR //!< SPM control and status register.



#elif defined(__ATmega1284P__) || defined(__AVR_ATmega1284P__)



#define SPMCR_REG SPMCSR //!< SPM control and status register.



#elif defined(__ATmega3290P__) || defined(__AVR_ATmega3290P__)



#define SPMCR_REG SPMCSR //!< SPM control and status register.



#else
#error "Unsupported device."
#endif
/*================================= TYEPDEFS         =========================*/

#if defined(__ICCAVR__)

#ifndef __HAS_ELPM__
typedef uint16_t sp_adr_t;
#else /* __HAS_ELPM__ */
typedef uint32_t sp_adr_t;
#endif /* __HAS_ELPM__ */



#elif defined(__GNUC__)

#ifndef LARGE_MEMORY
typedef uint16_t sp_adr_t;
#else /* LARGE_MEMORY */
typedef uint32_t sp_adr_t;
#endif /* LARGE_MEMORY */

#else
#error "Don't know your compiler."
#endif
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
#endif
/*EOF*/
