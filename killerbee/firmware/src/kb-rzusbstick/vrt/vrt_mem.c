// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements a dynamic memory allocation unit.
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
 * $Id: vrt_mem.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "compiler.h"
#include "vrt_mem.h"

//! \addtogroup grVRTMem
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
static uint8_t nmbr_of_partitions = 0;
static vrt_mem_partition_desc_t *first_partition;
/*================================= PROTOTYPES       =========================*/
bool vrt_mem_init(void) {
    nmbr_of_partitions = 0;
    first_partition = NULL;
    
	return true;
}


bool vrt_mem_add_partition(uint8_t *address, uint16_t block_size, uint8_t block_count, vrt_mem_partition_desc_t *desc) {
    /* Validate  function parameters. */
    if (NULL == address) { return false; }
    if (block_count < 2) { return false; }
    if (block_size < sizeof(void *)) { return false; }
    
    /* Protect the remaining code from interrupts. */
    ENTER_CRITICAL_REGION();
    
    /* Build list of free blocks. */
    void **link = (void **)address;
    uint8_t *block  = (uint8_t *)(address + block_size);
    
    for (uint8_t i = 0; i < (block_count - 1); i++) {
       *link  = (void *)block;
        link  = (void **)block;
        block = (uint8_t *)(block + block_size);
    }
    
    *link = (void *)NULL;
    
    /* Set-up the partition. */
    desc->block_size = block_size;
    desc->block_count = block_count;
    desc->free_block_count = block_count;
    desc->partition_start = address;
    desc->partition_end = address + (block_size * block_count) - 1;
    desc->free_list = address;
    
    /* Add to the list of partitions. The partitions are sorted by ascending block size. */
    if (0 == nmbr_of_partitions) {
        desc->next = (vrt_mem_partition_desc_t *)NULL;
        first_partition = desc;
    } else {
        vrt_mem_partition_desc_t *current_partition = first_partition;
        vrt_mem_partition_desc_t *previous_partition = first_partition;
        uint8_t partiotions_left = nmbr_of_partitions;
    
        while(0 != partiotions_left) {
            if((current_partition->block_size) >= block_size) {
                break;
            } else {
                previous_partition = current_partition;
                current_partition = current_partition->next;
                partiotions_left--;
            }
        }
        
        previous_partition->next = desc;
        desc->next = current_partition;
    }
    
    nmbr_of_partitions++;
    
    LEAVE_CRITICAL_REGION();
	return true;
}


void * vrt_mem_alloc(uint16_t block_size) {
    /* Validate function parameter. */
    if (0 == block_size) { return (void*)NULL; }
    /* Check that there is at least one partition available. */
    if (0 == nmbr_of_partitions) { return (void*)NULL; }
    
    /* Find the smallest block available. */
    ENTER_CRITICAL_REGION();
    vrt_mem_partition_desc_t *partition = first_partition;
    uint8_t partiotions_left = nmbr_of_partitions;
    
    bool partition_found = false;
    while((0 != partiotions_left) && (false == partition_found)) {
        if(((partition->block_size) >= block_size) && (0 != (partition->free_block_count))) {
            partition_found = true;
        } else {
            partition = partition->next;
            partiotions_left--;
        }
    }
    
    void *block = (void *)NULL;
    
    if (true == partition_found) {
        block = partition->free_list;
        partition->free_list = *(void **)block;
        (partition->free_block_count)--;
    }
    
    LEAVE_CRITICAL_REGION();
    
    return block;
}


void vrt_mem_free(void *block) {
    /* Verify that the function parameter is valid. */
    if (NULL == block) { return; }
    
    /* Check that there is at least one partition available. */
    if (0 == nmbr_of_partitions) { return; }
    
    /* Find partition where this block belongs. */
    ENTER_CRITICAL_REGION();
    vrt_mem_partition_desc_t *partition = first_partition;
    uint8_t partiotions_left = nmbr_of_partitions;
    
    bool partition_found = false;
    while((0 != partiotions_left) && (false == partition_found)) {
        void *start = partition->partition_start;
        void *end   = partition->partition_end;
        
        if((block >= start) && (block <= end)) {
            partition_found = true;
        } else {
            partition = partition->next;
            partiotions_left--;
        }
    }
    
    if (true == partition_found) {
        /* Check if this partitions is full or not. */
        if ((partition->free_block_count) >= (partition->block_count)) { return; }
        
        *(void **)block = partition->free_list;
        partition->free_list = block;
        (partition->free_block_count)++;
    }
    
    LEAVE_CRITICAL_REGION();
}
//! @}
/*EOF*/
