// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Header file for df.c
 *
 * \defgroup grAt45DLowLevel The AT45D Serial Flash Low Level Driver
 * \ingroup grLowLevelDrivers
 *
 * The AT45D is a family of high density serial Flash from Atmel.
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
 * $Id: df.h 41457 2008-05-09 08:15:04Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __df_H__
#define __df_H__

#include <stdbool.h>
#include <stdint.h>

#include "compiler.h"
#include "df_conf.h"

//! \addtogroup grAt45DLowLevel
//! @{
/*********************************************************************************************************
                                        Command interface
*********************************************************************************************************/

/// Read commands
//@{
#define FLASH_PAGE_READ				0xD2

#define CONT_ARRAY_READ_LF			0x03
#define CONT_ARRAY_READ_HF			0x0B

#define BUF1_READ_LF			    0xD1
#define BUF1_READ				    0xD4

#define BUF2_READ_LF			    0xD6
#define BUF2_READ				    0xD3
//@}

/// Program and erase commands
//@{
#define BUF1_WRITE					0x84
#define BUF2_WRITE					0x87
#define BUF1_TO_FLASH_AUTO_ERASE 	0x83
#define BUF2_TO_FLASH_AUTO_ERASE	0x86
#define BUF1_TO_FLASH 	            0x88
#define BUF2_TO_FLASH           	0x89

#define PAGE_ERASE		            0x81
#define BLOCK_ERASE 		        0x50
#define SECTOR_ERASE 		        0x7C
#define CHIP_ERASE 		            0xC794809A

#define FLASH_PAGE_PROG_BUF1		0x82
#define FLASH_PAGE_PROG_BUF2 		0x85

//@}

/// Protection and Security Commands
//@{
#define SECTOR_PROT_ENABLE   		0x3D2A7FA9
#define SECTOR_PROT_DISABLE   		0x3D2A7F9A
#define SECTOR_PROT_REG_ERASE  		0x3D2A7FCF
#define SECTOR_PROT_REG_PROG  		0x3D2A7FFC
#define SECTOR_PROT_REG_READ  		0x32
#define SECTOR_LOCKDOWN       		0x3D2A7F30
#define SECTOR_LOCKDOWN_REG_READ	0x35
#define SECTOR_LOCKDOWN_REG_PROG    0x9B000000

#define SECURITY_REG_PROG           0x77
#define SECURITY_REG_READ       	0x55
//@}


/// Additional Commands
//@{
#define FLASH_PAGE_MOVE_TO_BUF1     0x53
#define FLASH_PAGE_MOVE_TO_BUF2     0x55
#define FLASH_PAGE_COMPARE_BUF2     0x60
#define FLASH_PAGE_COMPARE_BUF1     0x61
#define FLASH_PAGE_AUTO_REWRT_BUF1  0x58
#define FLASH_PAGE_AUTO_REWRT_BUF2  0x59

#define DEEP_POWER_DOWN             0xB9
#define RESUME                      0xAB
#define STATUS_REG_READ             0xD7
#define MANUFACT_DEVICE_ID_READ     0x9F
//@}


/*! \brief Get current status of Data flash

 * \return                             Nothing
**/
uint8_t df_status(void);


//! \brief      type big enough to address the entrie data flash
typedef uint32_t    df_adr_t;

//! \brief      type big enough to hold the size of the entrie data flash
typedef uint32_t    df_size_t;


/*! \brief      Initiate a continuous read from a location in the DataFlash

 * \param[in]      PageAdr             Address of flash page where cont.read starts from
 * \param[in]      IntPageAdr          Internal page address where cont.read starts from

 * \return                             Nothing
**/
void df_read_enable(uint16_t PageAdr, uint16_t IntPageAdr);

/*! \brief      Read one or more bytes from one of the dataflash internal SRAM buffers, and puts read bytes into buffer pointed to by *Data
 */
void df_buffer_read(uint8_t BufferNo, uint16_t IntPageAdr, int NoOfBytes, uint8_t *Data);

/*! \brief      Read one byte from one of the dataflash internal SRAM buffers
 */
uint8_t df_buffer_read_byte(uint8_t BufferNo, uint16_t IntPageAdr);


/*! \brief      Read dataflash buffer

 * \param[in]      start_adr           Start address of read
 * \param[in]      count               Number of bytes read
 * \param[out]     data                Data buffer

 * \return                             Nothing
**/
df_size_t df_read(df_adr_t start_adr, df_size_t count, uint8_t *data);

df_size_t df_write_(df_adr_t start_adr, df_size_t count, uint8_t *data, bool increment);
//df_size_t df_write(df_adr_t start_adr, df_size_t count, uint8_t *data);
#define df_write(start_adr, count, data) df_write_(start_adr, count, data, true)
//df_size_t df_fill(df_adr_t start_adr, df_size_t count, uint8_t data);
#define df_fill(start_adr, count, data) df_write_(start_adr, count, data, false)


/*! \brief      Enable continous write functionality to one of the dataflash buffers buffers. NOTE : User must ensure that CS goes high to terminate this mode before accessing other dataflash functionalities

 * \param[in]      BufferNo            Decides usage of either buffer 1 or 2
 * \param[in]      IntPageAdr          Internal page address

 * \return                             Nothing
**/
void df_buffer_write_enable(uint8_t BufferNo, uint16_t IntPageAdr);

/*! \brief      Copy one or more bytes to one of the dataflash internal SRAM buffers from AVR SRAM buffer pointed to by *Data

 * \param[in]      BufferNo            Decides usage of either buffer 1 or 2
 * \param[in]      IntPageAdr          Internal page address
 * \param[in]      NoOfBytes           Number of bytes to be written
 * \param[in]      Data                Data to be written
 * \param[in]      Increment           Set to true if linear data should be written

 * \return                             Nothing
**/
void df_buffer_write(uint8_t BufferNo, uint16_t IntPageAdr, uint16_t NoOfBytes, uint8_t *Data, bool Increment);

/*! \brief      Copy one or more bytes to one of the dataflash internal SRAM buffers from an AVR SRAM loaction
 */
void df_buffer_fill(uint8_t BufferNo, uint16_t IntPageAdr, uint16_t NoOfBytes, uint8_t Data);

/*! \brief      Write one byte to one of the dataflash internal SRAM buffers
 */
void df_buffer_write_byte(uint8_t BufferNo, uint16_t IntPageAdr, uint8_t Data);




/*! \brief      Initialize Data flash interface/chip

 * \return                             Nothing
**/
int df_init(void);


/*! \brief      Transfer a page from flash to dataflash SRAM buffer

 * \param[in]      BufferNo            Decides usage of either buffer 1 or 2
 * \param[in]      PageAdr             Address of page to be transferred to buffer

 * \return                             Nothing
**/
void df_page_to_buffer(uint8_t BufferNo, uint16_t PageAdr);



/*! \brief      Transfer a page from dataflash SRAM buffer to flash

 * \param[in]      BufferNo            Decides usage of either buffer 1 or 2
 * \param[in]      PageAdr             Address of flash page to be programmed

 * \return                             Nothing
**/
void df_buffer_to_page(uint8_t BufferNo, uint16_t PageAdr);

/*! \brief
 * \return                             Size of the page (in bytes)
**/
int df_pagesize(void);

/*! \brief
 * \return                             Number of internal page address bits from look-up table
**/
int df_page_adr_bits(void);

/*! \brief
 * \param[in]      PageAdr             Address of flash page where cont.read starts from
 * \param[in]      IntPageAdr          Internal page address where cont.read starts from

 * \return                             Nothing
**/
void df_cont_read_enable(uint16_t PageAdr, uint16_t IntPageAdr);

/*! \brief
 * \return                             Nothing
**/
uint8_t df_ready(void);


/*! \brief      Low level function to initialize HW
*/
int df_hal_init(void);


/*! \brief      Low level function to read/write a byte on the dataflash interface bus
 */
int df_hal_rw(uint8_t *out_data, uint8_t *in_data, int count);
//! @}
#endif // __df_H__

/*EOF*/
