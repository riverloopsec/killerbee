// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Low level dataflash driver.
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
 * $Id: df.c 41375 2008-05-07 13:35:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "df.h"

//! \addtogroup grAt45DLowLevel
//! @{
/*========================= MACROS                   =========================*/
#define MAX_COMMAND_SIZE 8
/*========================= TYEPDEFS                 =========================*/
//! Type holding size and address bit count for data flash
typedef struct {
    int AdrBits;
    int Size;
} page_t;

//! Type holding key values for data flash
typedef struct {
    page_t Page;
    int Size;
    int Blocks;
    int Sectors;
} atsdf_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Look-up table for page size/internal address bits vs. data flash sizes
PROGMEM_DECLARE(static page_t sgAtsdfPageInfo[]) = {
    {9, 264},   // 512k
    {9, 264},   // 1M
    {9, 264},   // 2M
    {9, 264},   // 4M
    {9, 264},   // 8M
    {10, 528},  // 16M
    {10, 528},  // 32M
    {11, 1056}  // 64M
};

//! Key values for data flash 
static atsdf_t sgAtsdf;

//! Buffer for data flash commands
static uint8_t df_data_buffer[MAX_COMMAND_SIZE];

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int df_init(void)
{
    uint8_t StatusReg;
	uint8_t index_copy;

    df_hal_init();

    // Reset dataflash
    df_rst_assert();
    df_rst_release();
    
    // clear buffer
    for (int i=0; i<sizeof(df_data_buffer); ++i) {
        df_data_buffer[i] = 0x00;
    }
     
    // read status reg
	StatusReg = df_status();
	index_copy = ((StatusReg & 0x38) >> 3);	//get the size info from status register

    // save data flash parameters
	sgAtsdf.Page.AdrBits = sgAtsdfPageInfo[index_copy].AdrBits;
	sgAtsdf.Page.Size   = sgAtsdfPageInfo[index_copy].Size;
   
	return ((StatusReg & 0x3C) == 0x2C) ? 0 : -1;	    //return the read status register value
}

df_size_t df_read(df_adr_t start_adr, df_size_t count, uint8_t *data)
{
    df_size_t nmb = count;

    while (count){
        int temp = ((start_adr+count)/DF_BUFFER_SIZE != start_adr/DF_BUFFER_SIZE)? DF_BUFFER_SIZE - start_adr%DF_BUFFER_SIZE : count;
        df_page_to_buffer(1, (uint16_t)(start_adr/DF_BUFFER_SIZE));
        df_buffer_read(1, (uint16_t)(start_adr%DF_BUFFER_SIZE), temp, data);

        start_adr += temp;
        count -= temp;
        data += temp;
    }
    return nmb;
}

df_size_t df_write_(df_adr_t start_adr, df_size_t count, uint8_t *data, bool increment)
{
    df_size_t nmb = count;

    while (count){
        uint16_t temp = ((start_adr+count)/DF_BUFFER_SIZE != start_adr/DF_BUFFER_SIZE)? DF_BUFFER_SIZE - start_adr%DF_BUFFER_SIZE : count;
        df_page_to_buffer(1, (uint16_t)(start_adr/DF_BUFFER_SIZE));

        df_buffer_write_enable(1, (uint16_t)(start_adr/DF_BUFFER_SIZE));
        df_buffer_write(1, (uint16_t)(start_adr%DF_BUFFER_SIZE), temp, data, increment);
        df_buffer_to_page(1, (uint16_t)(start_adr/DF_BUFFER_SIZE));

        start_adr += temp;
        count -= temp;
        if (increment == true) {
            data += temp;
        }
    }
    return nmb;
}

uint8_t df_ready(void)
{
    df_data_buffer[0] = STATUS_REG_READ;
    df_data_buffer[1] = 0x00;
    
	df_cs_assert();							//to reset dataflash command decoder
	df_hal_rw(df_data_buffer, df_data_buffer, 2);			        //send status register read op-code
	df_cs_release();							//make sure to toggle CS signal in order

	return (uint8_t)(df_data_buffer[1] & (1<<7));				//return the ready/busy_ value
}

void df_cont_read_enable(uint16_t PageAdr, uint16_t IntPageAdr)
{
    df_data_buffer[0] = CONT_ARRAY_READ_LF;													//Continuous Array Read op-code
    df_data_buffer[1] = (unsigned char)(PageAdr >> (16 - sgAtsdf.Page.AdrBits));						//upper part of page address
    df_data_buffer[2] = (unsigned char)((PageAdr << (sgAtsdf.Page.AdrBits - 8))+ (IntPageAdr>>8));	//lower part of page address and MSB of int.page adr.
    df_data_buffer[3] = (unsigned char)(IntPageAdr);										//LSB byte of internal page address
    /*LSB bytes of internal page address
	* perform 4 dummy writes
	* in order to intiate DataFlash
	* address pointers
    */
	df_cs_assert();																//to reset dataflash command decoder
    df_hal_rw(df_data_buffer, df_data_buffer, 8);
	df_cs_release();																//make sure to toggle CS signal in order
}
void df_read_enable(uint16_t PageAdr, uint16_t IntPageAdr)
{
    df_data_buffer[0] = FLASH_PAGE_READ;													//Array Read op-code
    df_data_buffer[1] = (unsigned char)(PageAdr >> (16 - sgAtsdf.Page.AdrBits));						//upper part of page address
    df_data_buffer[2] = (unsigned char)((PageAdr << (sgAtsdf.Page.AdrBits - 8))+ (IntPageAdr>>8));	//lower part of page address and MSB of int.page adr.
    df_data_buffer[3] = (unsigned char)(IntPageAdr);										//LSB byte of internal page address
    /*LSB bytes of internal page address
	* perform 4 dummy writes
	* in order to intiate DataFlash
	* address pointers
    */
	df_cs_assert();																//to reset dataflash command decoder
    df_hal_rw(df_data_buffer, df_data_buffer, 8);
	df_cs_release();																//make sure to toggle CS signal in order
}

void df_buffer_read(uint8_t BufferNo, uint16_t IntPageAdr, int NoOfBytes, uint8_t *Data)
{

#ifdef USE_BUFFER2
    df_data_buffer[0] = (1 == BufferNo) ? BUF1_READ : BUF2_READ;
#else
    df_data_buffer[0] = BUF1_READ;
#endif
    df_data_buffer[1] = 0x00;						        //don't cares
    df_data_buffer[2] = (unsigned char)(IntPageAdr>>8);  //upper part of internal buffer address
    df_data_buffer[3] = (unsigned char)(IntPageAdr);	    //lower part of internal buffer address
    df_data_buffer[4] = 0x00;						        //don't cares
	df_cs_assert();								//to reset dataflash command decoder
    df_hal_rw(df_data_buffer, df_data_buffer, 5);
    df_hal_rw(&df_data_buffer[4], Data, NoOfBytes);
	df_cs_release();								//make sure to toggle CS signal in order
}

void df_buffer_write_enable(uint8_t BufferNo, uint16_t IntPageAdr)
{
#ifdef USE_BUFFER2
    df_data_buffer[0] = (1 == BufferNo) ? BUF1_WRITE : BUF2_WRITE;
#else
    df_data_buffer[0] = BUF1_WRITE;
#endif
    df_data_buffer[1] = 0x00;						        //don't cares
    df_data_buffer[2] = (unsigned char)(IntPageAdr>>8);  //upper part of internal buffer address
    df_data_buffer[3] = (unsigned char)(IntPageAdr);	    //lower part of internal buffer address
	df_cs_assert();								//to reset dataflash command decoder
    df_hal_rw(df_data_buffer, df_data_buffer, 4);
	df_cs_release();								//make sure to toggle CS signal in order
}

void df_buffer_write(uint8_t BufferNo, uint16_t IntPageAdr, uint16_t NoOfBytes, uint8_t *Data, bool Increment)
{
    uint8_t tmp;
#ifdef USE_BUFFER2
    df_data_buffer[0] = (1 == BufferNo) ? BUF1_WRITE : BUF2_WRITE;
#else
    df_data_buffer[0] = BUF1_WRITE;
#endif
    df_data_buffer[1] = 0x00;						        //don't cares
    df_data_buffer[2] = (unsigned char)(IntPageAdr>>8);  //upper part of internal buffer address
    df_data_buffer[3] = (unsigned char)(IntPageAdr);	    //lower part of internal buffer address
	df_cs_assert();								//to reset dataflash command decoder
    df_hal_rw(df_data_buffer, df_data_buffer, 4);
    for (int i=0; i<NoOfBytes; i++) {
        df_hal_rw(Data, &tmp, 1);			//write byte pointed at by *Data to dataflash buffer 1 location
        if (Increment == true) {
            Data++;						//point to next element in buffer
        }
    }
	df_cs_release();								//make sure to toggle CS signal in order
}

uint8_t df_status(void)
{
    df_data_buffer[0] = STATUS_REG_READ;
    df_data_buffer[1] = 0x00;

    
	df_cs_assert();							//to reset dataflash command decoder
	df_hal_rw(df_data_buffer, df_data_buffer, 2);			        //send status register read op-code
	df_cs_release();							//make sure to toggle CS signal in order

	return df_data_buffer[1];							//return the read status register value
}


int df_pagesize(void)
{
	return sgAtsdf.Page.Size;
}

int df_page_adr_bits(void)
{
	return sgAtsdf.Page.AdrBits;
}

void df_page_to_buffer(uint8_t BufferNo, uint16_t PageAdr)
{
#ifdef USE_BUFFER2
    df_data_buffer[0] = (1 == BufferNo) ? FLASH_PAGE_MOVE_TO_BUF1 : FLASH_PAGE_MOVE_TO_BUF2;
#else
    df_data_buffer[0] = FLASH_PAGE_MOVE_TO_BUF1;
#endif
    df_data_buffer[1] = (unsigned char)(PageAdr >> (16 - sgAtsdf.Page.AdrBits));	//upper part of page address
    df_data_buffer[2] = (unsigned char)(PageAdr << (sgAtsdf.Page.AdrBits - 8));	//lower part of page address
    df_data_buffer[3] = 0x00;										//don't cares
	df_cs_assert();								//to reset dataflash command decoder
    df_hal_rw(df_data_buffer, df_data_buffer, 4);
	df_cs_release();												//initiate the transfer
    
	while((df_status()&0xBC) != 0xAC){							    //monitor the status register, wait until busy-flag is high
        ;
    }
}

void df_buffer_to_page(uint8_t BufferNo, uint16_t PageAdr)
{
#ifdef USE_BUFFER2
    df_data_buffer[0] = (1 == BufferNo) ? BUF1_TO_FLASH_AUTO_ERASE : BUF2_TO_FLASH_AUTO_ERASE;
#else
    df_data_buffer[0] = BUF1_TO_FLASH_AUTO_ERASE;
#endif
    df_data_buffer[1] = (unsigned char)(PageAdr >> (16 - sgAtsdf.Page.AdrBits));	//upper part of page address
    df_data_buffer[2] = (unsigned char)(PageAdr << (sgAtsdf.Page.AdrBits - 8));	//lower part of page address
    df_data_buffer[3] = 0x00;										//don't cares
	df_cs_assert();								//to reset dataflash command decoder
    df_hal_rw(df_data_buffer, df_data_buffer, 4);
	df_cs_release();												//initiate the transfer
    
	while((df_status()&0xBC) != 0xAC){							    //monitor the status register, wait until busy-flag is high
        ;
    }
}
//! @}
/*EOF*/
