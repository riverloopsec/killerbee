// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Header file for the VRT MEM runtime framework.
 *
 * \defgroup grVRTMem The VRT Memory Allocation Unit
 * \ingroup grVRT
 *
 *  VRT Memory realizes a dynamic memory allocation unit based on a fragmentation-free
 *  memory manager. Memory is added to the manager as partitions that contains a
 *  cetain number of equally sized memory blocks. The number of partitions, 
 *  the block size and block count for each bag can be fine-tuned to match a 
 *  specific application's memory requirements...............
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
 * $Id: vrt_mem.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 #ifndef VRT_MEM_H
#define VRT_MEM_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

//! \addtogroup grVRTMem
//! @{
/*================================= MACROS           =========================*/
/*! \brief Convenience macro to allocate memory for a certain data type. */
#define MEM_ALLOC(data_type)             vrt_mem_alloc(sizeof(data_type))

/*! \brief Convenience macro to allocate a buffer of a certain data type and number of items. */
#define MEM_ALLOC_ARRAY(data_type, count) vrt_mem_alloc(sizeof(data_type) * (count))

/*! \brief Convenience macro to free up memory. */
#define MEM_FREE(ptr)                   vrt_mem_free((void *) (ptr))
/*================================= TYEPDEFS         =========================*/
typedef struct VRT_MEM_PARTITION_DESC_STRUCT {
    uint16_t block_size;  //!< Number of byte per block in this bag.
	uint8_t block_count; //!< Total number of blocks.
	uint8_t free_block_count;  //!< Number of free blocks in this bag.
	void *partition_start;  //!< Pointer to first byte of memory buffer.
    void *partition_end;  //!< Pointer to last byte of memory buffer.
    void *free_list; //!< List of free
    struct VRT_MEM_PARTITION_DESC_STRUCT *next; //!< Pointer to next memory partition
} vrt_mem_partition_desc_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function will set-up the VRT MEM module.
 *  \note This function must be called before any other function in the VRT MEM 
 *        module is called.
 *
 *  \retval true VRT MEM module successfully initialized.
 *  \retval false Will never happen.
 *
 *  \ingroup apiVRT_MEM
 */
bool vrt_mem_init(void);

/*! \brief This function will add a new partition to the memory manager.
 *
 *  \param address Pointer to the actual memory that will make up the partition.
 *  \param block_size Size in bytes of each memory block.
 *  \param block_count Number of blocks in the partition.
 *  \param desc Pointer to the partition descriptor that will store all the configuration
 *              data for this partition.
 *
 *  \retval true Partition added.
 *  \retval false Partition was not be added.
 *
 *  \ingroup apiVRT_MEM
 */
bool vrt_mem_add_partition(uint8_t *address, uint16_t block_size, uint8_t block_count, vrt_mem_partition_desc_t *desc);

/*! \brief This function is used to allocate a memory block of a given size.
 *
 *  \param block_size Size of the memory block that is to be allocated.
 *
 *  \returns Null is returned if it was not possible to allocate the requested block. 
 *           In any other case a pointer to the allocated memory block is returned.
 *
 *  \ingroup apiVRT_MEM
 */
void * vrt_mem_alloc(uint16_t block_size);

/*! \brief This function is used to free a memory block previously allocated.
 *
 *  \param block Pointer to the memory block that is to be returned.
 *
 *  \ingroup apiVRT_MEM
 */
void vrt_mem_free(void *block);
//! @}
#endif
/*EOF*/
